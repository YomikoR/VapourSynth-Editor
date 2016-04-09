#ifndef ALIGNED_VECTOR_H_INCLUDED
#define ALIGNED_VECTOR_H_INCLUDED

// Based on code from
// https://mastermind-strategy.googlecode.com/svn-history/r167/trunk/src/util/
// aligned_allocator.hpp

#include <cstdlib>
#include <memory>
#include <vector>

#define VSE_MEMORY_ALIGNMENT 32

namespace vsedit
{

	template <class T, size_t Alignment>
	struct aligned_allocator
		: public std::allocator<T>
	{
		typedef typename std::allocator<T>::size_type size_type;
		typedef typename std::allocator<T>::pointer pointer;
		typedef typename std::allocator<T>::const_pointer const_pointer;


		template <class U>
		struct rebind
		{
			typedef aligned_allocator<U, Alignment> other;
		};

		aligned_allocator() throw()
		{
		}

		aligned_allocator(const aligned_allocator & other) throw() :
			std::allocator<T>(other)
		{
		}

		template <class U>
		aligned_allocator(const aligned_allocator<U, Alignment> &) throw()
		{
		}

		~aligned_allocator() throw()
		{
		}

		pointer allocate(size_type n)
		{
			return allocate(n, const_pointer(0));
		}

		pointer allocate(size_type a_size, const_pointer a_hint)
		{
			(void)a_hint;

			void * p;
			#ifndef _WIN32
				if(posix_memalign(&p, Alignment, a_size * sizeof(T)) != 0)
					p = nullptr;
			#else
				p = _aligned_malloc(a_size * sizeof(T), Alignment);
			#endif
			if(!p)
				throw std::bad_alloc();
			return static_cast<pointer>(p);
		}

		void deallocate(pointer a_p, size_type a_size)
		{
			(void)a_size;

			#ifndef _WIN32
				free(a_p);
			#else
				_aligned_free(a_p);
			#endif
		}
	};

	template <class T1, size_t A1, class T2, size_t A2>
	bool operator == (const aligned_allocator<T1, A1> &,
		const aligned_allocator<T2,A2> &)
	{
		return true;
	}

	template <class T1, size_t A1, class T2, size_t A2>
	bool operator != (const aligned_allocator<T1, A1> &,
		const aligned_allocator<T2,A2> &)
	{
		return false;
	}

	template <class T>
	using aligned_vector =
		std::vector<T, aligned_allocator<T, VSE_MEMORY_ALIGNMENT> >;
}

#endif // ALIGNED_VECTOR_H_INCLUDED
