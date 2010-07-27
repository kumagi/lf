#include <boost/random.hpp>
#include <boost/functional/hash.hpp>
#include <boost/functional/hash_fwd.hpp>
#include <memory>
#include <functional>
#include <boost/functional.hpp>
#include <boost/interprocess/detail/atomic.hpp>
#include <boost/static_assert.hpp>
#include <stdio.h>

#if defined(__i386__)
inline void* cas_pointer(volatile void** ptr, void* comp, void* swap){
	BOOST_STATIC_ASSERT(sizeof(void*) == 4);
	return (void*)boost::interprocess::detail::atomic_cas32((volatile uint32_t*)ptr,(uint32_t)comp,(uint32_t)swap);
}
inline bool bool_cas_pointer(volatile void** ptr, void* comp, void* swap){
	return cas_pointer(ptr,comp,swap) != swap;
}
#elif  defined(__x86_64__)

#endif

namespace lockfree{

template<class T,
		class hash = boost::hash<T>,
		class comp = std::equal_to<T> >
//		class alloc = std::allocator<T> > 
class unordered_set{
	template<typename p>
	class markable_ptr{
	public:
		p* ptr_;
		markable_ptr(p* _ptr):ptr_(_ptr){}
		p* get()const {return ptr_;}
		bool attempt_mark(){
			if(ptr_ & 1){return false;}
			return cas_pointer(&ptr_,ptr_&~1,ptr_|1);
		}
	private:
		markable_ptr();
	};
	
	class node{
	public:
		const T t_;
		const uint32_t hash_;
		markable_ptr<node> next_;
		explicit node(const T& _t):t_(_t),hash_(hash_value(_t)),next_(NULL){}
		node(const T& _t, uint32_t _hash):t_(_t),hash_(_hash),next_(NULL){}
		~node(){
			delete next_.ptr();
		}
	};
	
	node*** shortcut;
	int bucket_power;
public:
	const static uint64_t max_memory = (uint64_t)1024*1024*1024*4;
	const static uint32_t onebucket = 32;
	const static uint32_t bucket_ptr_array = 128;
	unordered_set()
		:shortcut((node***)malloc(max_memory / (onebucket * bucket_ptr_array) * sizeof(node))),bucket_power(0)
	{
		for(int i=1;i<max_memory / (onebucket * bucket_ptr_array); i++){
			shortcut[i] = NULL;
		}
		shortcut[0] = (node**)malloc(bucket_ptr_array * sizeof(node*));
		for(int i=0;i<bucket_ptr_array;i++){
			shortcut[0][i] = new node(T(i), bit_inverse(i));
			if(i)insert_centinel(shortcut[0][i]);
		}
	}
	bool insert(const T& newobj){// multi thread free
		uint32_t hashed = hash_value(newobj);
		uint32_t array = (hashed % (bucket_ptr_array << bucket_power)) / bucket_ptr_array;
		uint32_t index = hashed % bucket_ptr_array;
		node* target_centinel = shortcut[array][index];
		if(target_centinel == NULL){
			node* new_centinel = new node(T(), bit_inverse(array * bucket_ptr_array + index));
			if(!bool_cas_pointer(target_centinel, NULL, new_centinel)){
				delete new_centinel;
			}else{
				recursive_insert_centinel(new_centinel);
			}
		}
	}
	bool remove(const T& obj){// multi thread freeunsigned int
		
	}
	void dump()const{
		const node* ptr = shortcut[0][0];
		while(ptr != NULL){
			fprintf(stderr,"%d,",ptr->t_);
			ptr = ptr->next_.get();
		}
	}
private:
	void recursive_insert_centinel(node* newcentinel){ // thread free
		uint32_t position = newcentinel->hash_;
		node* head = 
		
		}
	void insert_centinel(node* newnode)const{// not thread free
		node* pred = shortcut[0][0];
		node* curr = pred->next_.get();
		while(curr != NULL && curr->hash_ < newnode->hash_){
			pred = curr;
			curr = curr->next_.get();
		}
		newnode->next_ = curr;
		pred->next_ = newnode;
	}
	uint32_t bit_inverse(unsigned int bits)const{
		uint32_t ans = 0;
		uint32_t dst = 1 << (sizeof(int)*8 - 1) ;
		while(bits != 0){
			ans |= bits & 1 ? dst : 0;
			bits >>= 1;
			dst >>= 1;
		}
		return ans;
	}
};
}
int main(void){
	boost::mt11213b p;
	lockfree::unordered_set<int> a;
	p.seed(time(NULL));
	a.dump();
	printf("hello random:%d\n",boost::hash_value(p()));
	return 0;
}
