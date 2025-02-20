#include "base.hpp"

static
void* arena_allocator_func(
	void* impl,
	AllocatorMode op,
	void* old_ptr,
	Size old_size,
	Size size,
	Size align,
	U32* capabilities
){

	auto arena = (Arena*)impl;
	using M = AllocatorMode;
	using C = AllocatorCapability;

	switch (op) {
	case M::Query:
		*capabilities = U32(C::AlignAny) | U32(C::AllocAny) | U32(C::FreeAll);
		return nullptr;

	case M::Alloc:
		return arena->alloc(size, align);

	case M::Resize:
		return arena->resize_in_place(old_ptr, size);

	case M::Free:
		return nullptr;

	case M::FreeAll:
		arena->free_all();
		return nullptr;

	case M::Realloc:
		return arena->realloc(old_ptr, old_size, size, align);
	}

	return nullptr;
}

Allocator Arena::as_allocator(){
	Allocator alloc = {
		.data = this,
		.func = arena_allocator_func,
	};
	return alloc;
}

Arena Arena::from_buffer(Slice<U8> buf){
	PageBlock data = {
		.reserved = buf.len(),
		.commited = buf.len(),
		.pointer = buf.raw_data(),
	};

	Arena a = {
		.data = data,
		.offset = 0,
		.last_allocation = 0,
		.type = ArenaType::Buffer,
	};

	return a;
}

Arena Arena::make_virtual(Size reserve){
	auto data = PageBlock::make(reserve);

	Arena a = {
		.data = data,
		.offset = 0,
		.last_allocation = 0,
		.type = ArenaType::Virtual,
	};

	return a;
}

static
Uintptr arena_required_mem(Uintptr cur, Size count, Size align){
	ensure(mem_valid_alignment(align), "Alignment must be a power of 2");
	Uintptr aligned  = mem_align_forward_ptr(cur, align);
	Uintptr padding  = (Uintptr)(aligned - cur);
	Uintptr required = padding + count;
	return required;
}

void* Arena::alloc(Size size, Size align){
retry:
	Uintptr base = (Uintptr)data.pointer;
	Uintptr current = (Uintptr)base + (Uintptr)offset;

	Size available = data.commited - (current - base);
	Size required  = arena_required_mem(current, size, align);

	[[unlikely]] if(required > available){
		Size in_reserve = data.reserved - data.commited;
		Size diff = required - available;
		if(diff > in_reserve){
			return NULL; /* Out of memory */
		}
		else if(type == ArenaType::Virtual){
			if(data.push(required) == NULL){
				return NULL; /* Out of memory */
			}
			goto retry;
		}
	}

	offset += required;
	void* allocation = (U8*)data.pointer + (offset - size);
	last_allocation = (Uintptr)allocation;
	mem_set(allocation, 0, size);
	return allocation;
}

void* Arena::resize_in_place(void* ptr, Size new_size){
retry:
	if((Uintptr)ptr == last_allocation){
		Uintptr base    = Uintptr(data.pointer);
		Uintptr current = base + Uintptr(offset);
		Uintptr limit   = base + Uintptr(data.commited);

		Size last_allocation_size = current - last_allocation;
		if((current - last_allocation_size + new_size) > limit){
			if(type == ArenaType::Virtual){
				Size to_commit = (current - last_allocation_size + new_size) - limit;
				if(data.push(to_commit) != NULL){
					goto retry;
				}
			}

			return NULL; /* No space left*/
		}

		offset += new_size - last_allocation_size;
		return ptr;
	}

	return NULL;
}

void* Arena::realloc(void* ptr, Size old_size, Size new_size, Size align){
	void* new_ptr = this->resize_in_place(ptr, new_size);
	if(new_ptr == NULL){
		new_ptr = this->alloc(new_size, align);
		if(new_ptr != NULL){
			mem_copy_no_overlap(new_ptr, ptr, min(old_size, new_size));
		}
	}
	return new_ptr;
}

void Arena::free_all(){
	offset = 0;
}

void Arena::destroy(){
	this->free_all();
	if(type == ArenaType::Virtual){
		data.destroy();
	}
}

