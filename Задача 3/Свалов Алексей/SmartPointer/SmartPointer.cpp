#include "pch.h"
#include <iostream>


using namespace std;


template <class T>
class SmartPointer
{
private:
	T* ptr = NULL;
	size_t* copies_count;

public:

	explicit SmartPointer(T* v) : ptr(v)
	{
		copies_count = new size_t(0);
	}

	SmartPointer(const SmartPointer<T>& other)
	{
		Set(other);
	}

	void Set(const SmartPointer<T>& other)
	{
		Release();
		copies_count = other.copies_count;
		(*copies_count)++;
		ptr = other.ptr;
	}

	T& Get()
	{
		return *ptr;
	}

	void Release()
	{
		if (ptr != nullptr) {
			if (*(copies_count) == 0)
			{
				delete ptr;
				delete copies_count;
				copies_count = NULL;
				cout << "\nDelete Object\n";
			}
			else {
				(*copies_count)--;
				cout << "\nDelete Copy\n";
			}
		}
		ptr = nullptr;
	}

	SmartPointer& operator=(const SmartPointer<T>& other)
	{
		Release();
		Set(other);
		return *this;
	}

	T* operator->() const
	{
		return ptr;
	}

	T& operator*() const
	{
		return *ptr;
	}

	~SmartPointer()
	{
		Release();
	}

};

int main()
{
	SmartPointer<int> p(new int(5000));
	auto p1 = p;
	SmartPointer<int> pointer(new int(1000));
	SmartPointer<int> pointer1 = pointer;
	SmartPointer<int> pointer2 = pointer;
	cout << pointer1.Get() << "\n";
	cout << pointer2.Get() << "\n";
	cout << *p1 << "\n";
	p.Release();
	pointer.Release();
	pointer2.Release();
	pointer1.Set(SmartPointer<int>(new int(9999)));
	p1 = pointer1;
	return 0;
}