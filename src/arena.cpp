#include "base.hpp"

static
void* arena_allocator_func(
	void* impl,
	AllocatorMode op,
	void* old_ptr,
	isize old_size,
	isize size,
	isize align,
	u32* capabilities
){

	auto arena = (Arena*)impl;
	using M = AllocatorMode;
	using C = AllocatorCapability;

	switch (op) {
	case M::Query:
		*capabilities = u32(C::AlignAny) | u32(C::AllocAny) | u32(C::FreeAll);
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

Arena Arena::from_buffer(Slice<u8> buf){
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

Arena Arena::make_virtual(isize reserve){
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
uintptr arena_required_mem(uintptr cur, isize count, isize align){
	ensure(mem_valid_alignment(align), "Alignment must be a power of 2");
	uintptr aligned  = mem_align_forward_ptr(cur, align);
	uintptr padding  = (uintptr)(aligned - cur);
	uintptr required = padding + count;
	return required;
}

void* Arena::alloc(isize size, isize align){
retry:
	uintptr base = (uintptr)data.pointer;
	uintptr current = (uintptr)base + (uintptr)offset;

	isize available = data.commited - (current - base);
	isize required  = arena_required_mem(current, size, align);

	[[unlikely]] if(required > available){
		isize in_reserve = data.reserved - data.commited;
		isize diff = required - available;
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
	void* allocation = (u8*)data.pointer + (offset - size);
	last_allocation = (uintptr)allocation;
	mem_set(allocation, 0, size);
	return allocation;
}

void* Arena::resize_in_place(void* ptr, isize new_size){
retry:
	if((uintptr)ptr == last_allocation){
		uintptr base    = uintptr(data.pointer);
		uintptr current = base + uintptr(offset);
		uintptr limit   = base + uintptr(data.commited);

		isize last_allocation_size = current - last_allocation;
		if((current - last_allocation_size + new_size) > limit){
			if(type == ArenaType::Virtual){
				isize to_commit = (current - last_allocation_size + new_size) - limit;
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

void* Arena::realloc(void* ptr, isize old_size, isize new_size, isize align){
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

