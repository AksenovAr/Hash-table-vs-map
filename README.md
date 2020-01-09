### Hash Map priority example

Show how to hash map find data faster than assotiative container

### Shared mutex - solution for reader - writer problem 

### Code example
```
std::shared_timed_mutex m;
my_data_structure data;

void reader(){
    std::shared_lock<std::shared_timed_mutex> lk(m);
    read_something(data);
}

void writer(){
    std::lock_guard<std::shared_timed_mutex> lk(m);
    update(data);
}
```
