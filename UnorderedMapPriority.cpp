#pragma gcc optimize( -ofast )
#include <iostream>
#include <unordered_map>
#include <map>
#include <shared_mutex>
#include <utility>
#include <thread>
#include <iostream>
#include <chrono>
#include <assert.h>

using namespace std;
using namespace std::chrono;

// Enum which class we should create
enum mapType
{
    ORDINARY = 0,
    UNORDERED
};

//--------------------- Interface class  -----------------------

template <typename KEY, typename VALUE>
class CInterface
{
  public:
    CInterface(){};
    virtual ~CInterface(){};
    virtual bool add(const KEY& key, const VALUE& value) = 0;
    virtual bool removeValue(const KEY& key) = 0;
    virtual bool getValue(const KEY& key, VALUE& val) = 0;
    virtual int isExists(const KEY& key) = 0;
    virtual void mapClear();
};

//--------------------- Ordinary map start -----------------------

template <typename KEY, typename VALUE>
class OrdinaryMapDateBase : public CInterface<KEY, VALUE>
{
  public:
    OrdinaryMapDateBase(){};
    virtual ~OrdinaryMapDateBase()
    {
        _map.clear();
    };
    virtual bool add(const KEY& key, const VALUE& value);
    virtual bool removeValue(const KEY& key);
    virtual bool getValue(const KEY& key, VALUE& val);
    virtual int isExists(const KEY& key);
    virtual void mapClear();
public:
    std::map<KEY, VALUE> _map;
};

template <class KEY, class VALUE>
void OrdinaryMapDateBase<KEY, VALUE>::mapClear()
{
    _map.clear();
}

template <class KEY, class VALUE>
int OrdinaryMapDateBase<KEY, VALUE>::isExists( const KEY& key )
{
    auto iCount = _map.count(key);
    return iCount;
}

template <typename KEY, typename VALUE>
bool OrdinaryMapDateBase<KEY, VALUE>::add(const KEY& key, const VALUE& value)
{
    bool bRes = false;
    auto itInner = _map.find(key);
    if (itInner != _map.end())
    {
        //already exist
        itInner->second = value;
    }
    else
    {
        //element is not exist
        _map.insert(std::make_pair(key,value));
        bRes = true;
    }
    return bRes;
}

template <typename KEY, typename VALUE>
bool OrdinaryMapDateBase<KEY, VALUE>::removeValue(const KEY& key)
{
    bool bRes = false;
    auto it = _map.find(key);
    if (it != _map.end())
    {
        _map.erase(it);
        bRes = true;
    }
    return bRes;
}

template <typename KEY, typename VALUE>
bool OrdinaryMapDateBase<KEY, VALUE>::getValue(const KEY& key, VALUE& val)
{
    bool bRes = false;
    auto it = _map.find(key);
    if (it != _map.end())
    {
        bRes = true;
        val = it->second;
    }
    return bRes;
}

//--------------------- Ordinary map end -----------------------
//--------------------- Unordered map start -----------------------

template <typename KEY, typename VALUE>
class UnorderedMapDateBase : public CInterface<KEY, VALUE>
{
  public:
    UnorderedMapDateBase(){};
    virtual ~UnorderedMapDateBase()
    {
        _map.clear();
    };
    virtual bool add(const KEY& key, const VALUE& value);
    virtual bool removeValue(const KEY& key);
    virtual bool getValue(const KEY& key, VALUE& val);
    virtual int isExists(const KEY& key);
    virtual void mapClear();

public:
    std::unordered_map<KEY, VALUE> _map;
};

template <class KEY, class VALUE>
void UnorderedMapDateBase<KEY, VALUE>::mapClear()
{
    _map.clear();
}

template <class KEY, class VALUE>
int UnorderedMapDateBase<KEY, VALUE>::isExists( const KEY& key )
{
    return _map.count(key);
}

template <typename KEY, typename VALUE>
bool UnorderedMapDateBase<KEY, VALUE>::add(const KEY& key, const VALUE& value)
{
    bool bRes = false;
    auto itInner = _map.find(key);
    if (itInner != _map.end())
    {
        //already exist
        itInner->second = value;
    }
    else
    {
        //element does not exist
        _map.insert(std::make_pair(key,value));
        bRes = true;
    }
    return bRes;
}

template <typename KEY, typename VALUE>
bool UnorderedMapDateBase<KEY, VALUE>::removeValue(const KEY& key)
{
    bool bRes = false;
    auto it = _map.find(key);
    if (it != _map.end())
    {
        _map.erase(it);
        bRes = true;
    }
    return bRes;
}

template <typename KEY, typename VALUE>
bool UnorderedMapDateBase<KEY, VALUE>::getValue(const KEY& key, VALUE& val)
{
    bool bRes = false;
    auto it = _map.find(key);
    if (it != _map.end())
    {
        bRes = true;
        val = it->second;
    }
    return bRes;
}

//------------------------ Unordered map end --------------------------

template <typename KEY, typename VALUE>
class mymap
{
public:
    mymap( mapType mType )
    {
        m_pInterface = nullptr;
        if (mType == mapType::ORDINARY)
        {
            m_pInterface = new OrdinaryMapDateBase<KEY, VALUE>();
        }
        else
        {
            m_pInterface = new UnorderedMapDateBase<KEY, VALUE>();
        }
    };
    mymap(const mymap&) = delete;
    mymap(mymap&&) = delete;
    mymap& operator = (const mymap&) = delete;
    mymap& operator = (mymap&&) = delete;
    ~mymap()
    {
        if ( m_pInterface != nullptr) delete m_pInterface;
    };

private:
    //we should use c++14 for shared_timed_mutex
    std::shared_timed_mutex _mMutex;
    CInterface<KEY, VALUE>* m_pInterface;

public:
    bool add(const KEY& key, const VALUE& value);
    bool removeValue(const KEY& key);
    bool getValue(const KEY& key, VALUE& val);
    //We can check collision by this function
    int isExists(const KEY& key);
    void mapClear();
};

template <class KEY, class VALUE>
int mymap<KEY, VALUE>::isExists( const KEY& key )
{
    std::shared_lock<std::shared_timed_mutex> readerLock(_mMutex);
    return m_pInterface->isExists(key);;
}

template <typename KEY, typename VALUE>
bool mymap<KEY, VALUE>::add(const KEY& key, const VALUE& value)
{
    std::lock_guard<std::shared_timed_mutex> writerLock(_mMutex);
    return m_pInterface->add(key, value);
}

template <typename KEY, typename VALUE>
bool mymap<KEY, VALUE>::removeValue(const KEY& key)
{
    std::lock_guard<std::shared_timed_mutex> writerLock(_mMutex);
    return m_pInterface->removeValue(key);
}

template <typename KEY, typename VALUE>
bool mymap<KEY, VALUE>::getValue(const KEY& key, VALUE& val)
{
    std::shared_lock<std::shared_timed_mutex> readerLock(_mMutex);
    return m_pInterface->getValue(key, val);
}

template <typename KEY, typename VALUE>
class TestMyMap
{
        public :
        TestMyMap( mymap<KEY, VALUE>* pMap )
        {
                m_pMap = pMap;
        }

        ~TestMyMap(){}
        void TestMainFunction()
        {
                 std::cout << "\nMain functionality is testing\n";
                 m_pMap->add("Scott", 1968);
                 m_pMap->add("Jhon", 1972);
                 int year;
                 m_pMap->getValue("Jhon", year );
                 //std::cout << "\nYear == " << year;
                 m_pMap->removeValue("Jhon");
                 bool res = m_pMap->getValue("Jhon", year );
                 if ( res == true)
                 {
                        assert (false);
                 }
                 m_pMap->add("Scott", 1977);
                 res = m_pMap->getValue("Scott", year );
                 if ( year != 1977 )
                 {
                        assert(false);
                 }
                 std:: cout << "Main functionality is OK.\n";
        }

        int TestMultiThreading()
        {
                high_resolution_clock::time_point t1 = high_resolution_clock::now();
                std::cout << "\nStart multi threading testing\n";
                std::thread w1([=]
                {
                        for ( int i = 0; i < 100000; ++i )
                        {

                                string str = std::to_string(i);
                                m_pMap->add(str, rand() % 10000 );
                        }

                });

                std::thread r1([=]
                {
                        for  ( int i = 0; i < 10000 ; ++i)
                        {
                                int iKey = rand() % 10000;
                                int iYear;
                                m_pMap->getValue(std::to_string(iKey), iYear);
                        }

                });

                std::thread r2([=]
                {
                        for  ( int i = 0; i < 10000 ; ++i)
                        {
                                int iKey = rand() % 10000;
                                int iYear;
                                m_pMap->getValue(std::to_string(iKey), iYear);
                        }

                });

                std::thread w2([=]
                {
                        for ( int i = 0; i < 10000; ++i )
                        {

                                string str = std::to_string(rand() % 10000);
                                m_pMap->removeValue(str);
                        }

                });

                std::thread r3([=]
                {
                        for  ( int i = 0; i < 10000 ; ++i)
                        {
                                int iKey = rand() % 10000;
                                int iYear;
                                m_pMap->getValue(std::to_string(iKey), iYear);
                        }

                });

                std::thread r4([=]
                {
                        for  ( int i = 0; i < 10000 ; ++i)
                        {
                                int iKey = rand() % 10000;
                                int iYear;
                                m_pMap->getValue(std::to_string(iKey), iYear);
                        }

                });

                std::thread r5([=]
                {
                        for  ( int i = 0; i < 10000 ; ++i)
                        {
                                int iKey = rand() % 10000;
                                int iYear;
                                m_pMap->getValue(std::to_string(iKey), iYear);
                        }

                });

                r1.join();
                r2.join();
                r3.join();
                r4.join();
                r5.join();
                w1.join();
                w2.join();

                high_resolution_clock::time_point t2 = high_resolution_clock::now();
                auto duration = duration_cast<microseconds>( t2 - t1 ).count();
                return duration;
        }

        mymap<KEY, VALUE>* m_pMap;
};

int main()
{
        // your code goes here
        mymap<std::string, int> tmpMap(mapType::UNORDERED);
        TestMyMap<std::string, int> TestObj( &tmpMap );
        std::cout << "Unordered map main functionality test is started.";
        TestObj.TestMainFunction();
        int iUnorderedDuration = TestObj.TestMultiThreading();
        std::cout << "\nUnordered map duration is :" << iUnorderedDuration << std::endl;

    mymap<std::string, int> ordinaryMap(mapType::ORDINARY);
        TestMyMap<std::string, int> ordinaryTestObj( &ordinaryMap );
        std::cout << "\nMap main functionality test is started.";
        ordinaryTestObj.TestMainFunction();
        int iOrdinaryDuration = ordinaryTestObj.TestMultiThreading();
        std::cout << "\nOrdinary map duration is :" << iOrdinaryDuration << std::endl;
    std::cout << "\nUnordered map faster than ordered map in :" <<  static_cast<double> ( iOrdinaryDuration ) /  iUnorderedDuration  << " times." << std::endl;
    std::cout << "\nAll tests were finished" << std::endl;
        return 0;
}

