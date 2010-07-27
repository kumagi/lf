#include <boost/random.hpp>
#include <assert.h>
#include <time.h>
namespace lockfree{

template<class key, class value, int maxlevel = 8>
class skiplist{
public:// objects
  template<class compatible_key, class compatible_value, int nodelevel = 8>
  class node{
  private:
    uint32_t max_level;
    std::vector<node<compatible_key, compatible_value, nodelevel>*> next; // level
    friend class skiplist;
  public:
    const compatible_key first;
    compatible_value second;
    node(const compatible_key& k, const compatible_value& v, uint32_t m=nodelevel)
      :first(k),second(v),max_level(m){
    }
  };
private:
  typedef node<const key,value> Node;
  struct iterator{
    Node* ptr_;
    iterator(Node* const ptr):ptr_(ptr){}
    bool operator==(const iterator& rhs)const{
      return ptr_ == rhs.ptr_;
    }
    iterator& operator=(const iterator& rhs){ptr_ = rhs.ptr_;}
    Node* operator->(){return ptr_;}
    Node& operator*(){return *ptr_;}
    const Node* operator->()const{return ptr_;}
    const Node& operator*()const{return *ptr_;}
  };
  const key min_;
  const key max_;
  Node head,tail;
  
  uint32_t rand_seed;
public:
  skiplist(const key& min, const key& max, const value& v = value())
    :min_(min),max_(max),
     head(min,v,maxlevel),
     tail(max,v,maxlevel),
     rand_seed(time(NULL))
  {
    head.next.reserve(maxlevel);
    tail.next.reserve(maxlevel);
    for(int i=0;i<maxlevel;i++){
      head.next[i] = &tail;
      tail.next[i] = NULL;
    }
  }
  bool insert(const std::pair<key,value>& kvp){
    const uint32_t random = rand_r(&rand_seed);
    rand_seed = random;
    const int level = random % maxlevel;
    Node* const newnode =  new Node(kvp.first,kvp.second, level);
    while(1){
      std::vector<Node*> pred(maxlevel);
      std::vector<Node*> succ(maxlevel);
      find(kvp.first, level, &pred,&succ);
      for(int i=0;i<level;i++){
	newnode->next[i] = succ[i];
      }
      for(int i=0;i<level;i++){
	cas(pred[i],succ[i],newnode);
      }
    }
  }
  iterator find(const key& k){
    Node* ptr = &head;
    int level = maxlevel-1;
    while(level >= 0){
      assert(ptr->next[level]);
      if(ptr->next[level]->first < k){
	ptr = ptr->next[level];
      }else if(ptr->next[level]->first == k){
	return iterator(ptr->next[level]);
      }else level--;
    }
    return end();
  }
  iterator end()const{
    return iterator(NULL);
  }
};



}// namespace lockfree
