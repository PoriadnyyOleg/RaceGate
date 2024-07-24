/*
	LinkedList.h - V1.1 - Generic LinkedList implementation
	Works better with FIFO, because LIFO will need to
	search the entire List to find the last one;

	For instructions, go to https://github.com/ivanseidel/LinkedList

	Created by Ivan Seidel Gomes, March, 2013.
	Released into the public domain.
*/


#ifndef LinkedList_h
#define LinkedList_h

#include <stddef.h>

template<class T>
struct ListNode
{
	T data;
	ListNode<T> *next;
};

template <typename T>
class List{

protected:
	int _size;
	ListNode<T> *root;
	ListNode<T>	*last;

	// Helps "get" method, by saving last position
	ListNode<T> *lastNodeGot;
	int lastIndexGot;
	// isCached should be set to FALSE
	// everytime the list suffer changes
	bool isCached;

	ListNode<T>* getNode(int index);

	ListNode<T>* findEndOfSortedString(ListNode<T> *p, int (*cmp)(T &, T &));

public:
	List();
	List(int sizeIndex, T _t); //initiate list size and default value

   T* getFirst();
     T* getLast();
	virtual ~List();

	/*
		Returns current size of List
	*/
	virtual int getSize();
	/*
		Adds a T object in the specified index;
		Unlink and link the List correcly;
		Increment _size
	*/
	virtual bool add(int index, T);
	/*
		Adds a T object in the end of the List;
		Increment _size;
	*/
	virtual bool add(T);
	/*
		Adds a T object in the start of the List;
		Increment _size;
	*/
	virtual bool unshift(T);
	/*
		Set the object at index, with T;
	*/
	virtual bool set(int index, T);
	/*
		Remove object at index;
		If index is not reachable, returns false;
		else, decrement _size
	*/
	virtual T remove(int index);
	/*
		Remove last object;
	*/
	virtual T pop();
	/*
		Remove first object;
	*/
	virtual T shift();
	/*
		Get the index'th element on the list;
		Return Element if accessible,
		else, return false;
	*/
	virtual T get(int index);

	/*
		Clear the entire array
	*/
	virtual void clear();

	/*
		Sort the list, given a comparison function
	*/
	virtual void sort(int (*cmp)(T &, T &));

		// add support to array brakets [] operator
	inline T& operator[](int index); 
	inline T& operator[](size_t& i) { return this->get(i); }
  	inline const T& operator[](const size_t& i) const { return this->get(i); }

};

// Initialize List with false values
template<typename T>
List<T>::List()
{
	root=NULL;
	last=NULL;
	_size=0;

	lastNodeGot = root;
	lastIndexGot = 0;
	isCached = false;
}

// Clear Nodes and free Memory
template<typename T>
List<T>::~List()
{
	ListNode<T>* tmp;
	while(root!=NULL)
	{
		tmp=root;
		root=root->next;
		delete tmp;
	}
	last = NULL;
	_size=0;
	isCached = false;
}

/*
	Actualy "logic" coding
*/
template<typename T>
 T* List<T>::getFirst(){

	//ListNode<T>* current = root;

		return &root->data;
	
}
template<typename T>
 T* List<T>::getLast(){



		return &last->data;
	
}

template<typename T>
ListNode<T>* List<T>::getNode(int index){

	int _pos = 0;
	ListNode<T>* current = root;

	// Check if the node trying to get is
	// immediatly AFTER the previous got one
	if(isCached && lastIndexGot <= index){
		_pos = lastIndexGot;
		current = lastNodeGot;
	}

	while(_pos < index && current){
		current = current->next;

		_pos++;
	}

	// Check if the object index got is the same as the required
	if(_pos == index){
		isCached = true;
		lastIndexGot = index;
		lastNodeGot = current;

		return current;
	}

	return NULL;
}

template<typename T>
int List<T>::getSize(){
	return _size;
}

template<typename T>
List<T>::List(int sizeIndex, T _t){
	for (int i = 0; i < sizeIndex; i++){
		add(_t);
	}
}

template<typename T>
bool List<T>::add(int index, T _t){

	if(index >= _size)
		return add(_t);

	if(index == 0)
		return unshift(_t);

	ListNode<T> *tmp = new ListNode<T>(),
				 *_prev = getNode(index-1);
	tmp->data = _t;
	tmp->next = _prev->next;
	_prev->next = tmp;

	_size++;
	isCached = false;

	return true;
}

template<typename T>
bool List<T>::add(T _t){

	ListNode<T> *tmp = new ListNode<T>();
	tmp->data = _t;
	tmp->next = NULL;
	
	if(root){
		// Already have elements inserted
		last->next = tmp;
		last = tmp;
	}else{
		// First element being inserted
		root = tmp;
		last = tmp;
	}

	_size++;
	isCached = false;

	return true;
}

template<typename T>
bool List<T>::unshift(T _t){

	if(_size == 0)
		return add(_t);

	ListNode<T> *tmp = new ListNode<T>();
	tmp->next = root;
	tmp->data = _t;
	root = tmp;
	
	_size++;
	isCached = false;
	
	return true;
}


template<typename T>
T& List<T>::operator[](int index) {
	return getNode(index)->data;
}

template<typename T>
bool List<T>::set(int index, T _t){
	// Check if index position is in bounds
	if(index < 0 || index >= _size)
		return false;

	getNode(index)->data = _t;
	return true;
}

template<typename T>
T List<T>::pop(){
	if(_size <= 0)
		return T();
	
	isCached = false;

	if(_size >= 2){
		ListNode<T> *tmp = getNode(_size - 2);
		T ret = tmp->next->data;
		delete(tmp->next);
		tmp->next = NULL;
		last = tmp;
		_size--;
		return ret;
	}else{
		// Only one element left on the list
		T ret = root->data;
		delete(root);
		root = NULL;
		last = NULL;
		_size = 0;
		return ret;
	}
}

template<typename T>
T List<T>::shift(){
	if(_size <= 0)
		return T();

	if(_size > 1){
		ListNode<T> *_next = root->next;
		T ret = root->data;
		delete(root);
		root = _next;
		_size --;
		isCached = false;

		return ret;
	}else{
		// Only one left, then pop()
		return pop();
	}

}

template<typename T>
T List<T>::remove(int index){
	if (index < 0 || index >= _size)
	{
		return T();
	}

	if(index == 0)
		return shift();
	
	if (index == _size-1)
	{
		return pop();
	}

	ListNode<T> *tmp = getNode(index - 1);
	ListNode<T> *toDelete = tmp->next;
	T ret = toDelete->data;
	tmp->next = tmp->next->next;
	delete(toDelete);
	_size--;
	isCached = false;
	return ret;
}


template<typename T>
T List<T>::get(int index){
	ListNode<T> *tmp = getNode(index);

	return (tmp ? tmp->data : T());
}

template<typename T>
void List<T>::clear(){
	while(getSize() > 0)
		shift();
}

template<typename T>
void List<T>::sort(int (*cmp)(T &, T &)){
	if(_size < 2) return; // trivial case;

	for(;;) {	

		ListNode<T> **joinPoint = &root;

		while(*joinPoint) {
			ListNode<T> *a = *joinPoint;
			ListNode<T> *a_end = findEndOfSortedString(a, cmp);
	
			if(!a_end->next	) {
				if(joinPoint == &root) {
					last = a_end;
					isCached = false;
					return;
				}
				else {
					break;
				}
			}

			ListNode<T> *b = a_end->next;
			ListNode<T> *b_end = findEndOfSortedString(b, cmp);

			ListNode<T> *tail = b_end->next;

			a_end->next = NULL;
			b_end->next = NULL;

			while(a && b) {
				if(cmp(a->data, b->data) <= 0) {
					*joinPoint = a;
					joinPoint = &a->next;
					a = a->next;	
				}
				else {
					*joinPoint = b;
					joinPoint = &b->next;
					b = b->next;	
				}
			}

			if(a) {
				*joinPoint = a;
				while(a->next) a = a->next;
				a->next = tail;
				joinPoint = &a->next;
			}
			else {
				*joinPoint = b;
				while(b->next) b = b->next;
				b->next = tail;
				joinPoint = &b->next;
			}
		}
	}
}

template<typename T>
ListNode<T>* List<T>::findEndOfSortedString(ListNode<T> *p, int (*cmp)(T &, T &)) {
	while(p->next && cmp(p->data, p->next->data) <= 0) {
		p = p->next;
	}
	
	return p;
}

#endif



/*template<typename T>
class List{
  public:
  List();
  ~List();

int getSize(){
  return size;
};
     bool dataAsObject=true;
   void popFront(bool toOut = false );// удаление первого элемента
   T * popBack(bool toOut);         // удалить последний элемент
   T * removeAt(int index, bool toOut = false);// удалить по индексу. toOut - если True, то не удаляется из ОЗУ, а возвращается из запроса




  void pushBack(T data){    //добавить в конец
    if (head ==nullptr){
    head = new Node<T>(data);  
      } else {
      Node <T> * current = this->head;
      while (current->pNext !=nullptr){
        current=current->pNext;
      }
      current->pNext=new Node<T>(data);
    } 
   size++;
  };

  T& operator[](const int index){
      Node<T> * current = this->head;
      int counter=0;
      while (current!=nullptr){
        if (counter==index){
          return current->data;
        }
        current=current->pNext;
        counter++;
      }
  };
  
   T getLast(){
       Node<T>* p = head;
       while (p->pNext != NULL)
           p = p->pNext;
       return p->data;


   };
    T getFirst(){// а надо?
        if (head)
            return head->data;
       // else return nullptr;

   };

 void clear();
  private:
    template<typename U>
     class Node{
       public:
       Node * pNext;
       T data;

       Node(T data=T(), Node * pNext= nullptr){
         this->data = data;
         this->pNext=pNext;
       }
     };
  int size;
  Node<T> *head;

};


template<typename T>
List<T>::List(){
  size=0;
  head=nullptr;
};



template <typename T>
void List<T>::popFront(bool toOut){ // удаление первого элемента
   
    Node<T> *temp = head; // head тут у нас не объект (!), а УКАЗАТЕЛЬ на УЗЕЛ, у которого поле data - это тоже указатель на другой объект другого класса
    //T *data = head->data; // указатель на объект data (ведь дата у нас не просто данные, а УКАЗАТЕЛЬ на объект, и этот объект тоже надо удолять из ОЗУ)
   
    head = head->pNext;

    size--;
   /* if (toOut){    
        return data;      
    }
    delete data;
    delete temp; // очистка ОЗУ от объекта data класса <T>  
    return nullptr;
    
};
template <typename T>
void List<T>::clear(){ // очистка всего списка
    int s = this->size;
    while(s){
        popFront(false); // удалить без возвращения удаляемого элемента
    }
};
// // деструктор
template <typename T>
List<T>::~List()
{
    //Serial << "Вызвался дестркутор объекта класса List" << endl;
    clear();
}



template <typename T>
T * List<T>::removeAt(int index, bool toOut){ // // удалить по индексу. toOut - если True, то не удаляется из ОЗУ, а возвращается из запроса
    if(index == 0){
        popFront(toOut);
    } else {
        Node<T> *prev = this->head;
        for (int i = 0; i < index - 1; i++)
        {
            prev = prev->pNext;
        }
        Node<T> *toDelete = prev->pNext;
        prev->pNext = toDelete->pNext;

        size--;

        if (toOut){
            T * answer = toDelete->data; // указатель на данные
           
            if (dataAsObject) { // удалить просто узел
                delete toDelete;
            }
            return answer;  // вернуть указатель на данные
        } else {
            // удалить из ОЗУ объект data по его указателю в узле toDelete
            if (dataAsObject){  delete toDelete->data; }          
            delete toDelete;    // удалить из ОЗУ просто узел
            return nullptr;
        }
    }    
}

template <typename T>
T * List<T>::popBack(bool toOut){ // удалить последний элемент
    return removeAt(size-1, toOut);
}
*/