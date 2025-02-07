#include "base.hpp"

Arena arena_from_buffer(Slice<U8> buf){
	PageBlock data = {
		.reserved = len(buf),
		.commited = len(buf),
		.pointer = raw_data(buf),
	};

	Arena a = {
		.data = data,
		.offset = 0,
		.last_allocation = 0,
		.type = ArenaType::Buffer,
	};

	return a;
}

Arena arena_create_virtual(Size reserve){
	auto data = page_block_create(reserve);

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

void* mem_alloc(Arena* a, Size size, Size align){
retry:
	Uintptr base = (Uintptr)a->data.pointer;
	Uintptr current = (Uintptr)base + (Uintptr)a->offset;

	Size available = a->data.commited - (current - base);
	Size required  = arena_required_mem(current, size, align);

	[[unlikely]] if(required > available){
		Size in_reserve = a->data.reserved - a->data.commited;
		Size diff = required - available;
		if(diff > in_reserve){
			return NULL; /* Out of memory */
		}
		else if(a->type == ArenaType::Virtual){
			if(page_block_push(&a->data, required) == NULL){
				return NULL; /* Out of memory */
			}
			goto retry;
		}
	}

	a->offset += required;
	void* allocation = (U8*)a->data.pointer + (a->offset - size);
	a->last_allocation = (Uintptr)allocation;
	return allocation;
}

void* mem_resize_in_place(Arena* a, void* ptr, Size new_size){
retry:
	if((Uintptr)ptr == a->last_allocation){
		Uintptr base    = Uintptr(a->data.pointer);
		Uintptr current = base + Uintptr(a->offset);
		Uintptr limit   = base + Uintptr(a->data.commited);

		Size last_allocation_size = current - a->last_allocation;
		if((current - last_allocation_size + new_size) > limit){
			if(a->type == ArenaType::Virtual){
				Size to_commit = (current - last_allocation_size + new_size) - limit;
				if(page_block_push(&a->data, to_commit) != NULL){
					goto retry;
				}
			}

			return NULL; /* No space left*/
		}

		a->offset += new_size - last_allocation_size;
		return ptr;
	}

	return NULL;
}

void* mem_realloc(Arena* a, void* ptr, Size old_size, Size new_size, Size align){
	void* new_ptr = mem_resize_in_place(a, ptr, new_size);
	if(new_ptr == NULL){
		new_ptr = mem_alloc(a, new_size, align);
		if(new_ptr != NULL){
			mem_copy_no_overlap(new_ptr, ptr, min(old_size, new_size));
		}
	}
	return new_ptr;
}

void mem_free_all(Arena* a){
	a->offset = 0;
}

void arena_destroy(Arena* a){
	mem_free_all(a);
	if(a->type == ArenaType::Virtual){
		page_block_destroy(&a->data);
	}
}

