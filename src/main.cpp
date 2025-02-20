#include "base.hpp"
// #include "debug_print.cpp"

template<typename K, typename V>
std::ostream& operator<<(std::ostream& os, Map<K, V> map){
	for(Size i = 0; i < map.capacity; i ++){
		auto head = &map.base_slots[i];

		os << "[" << i << "]\t";
		if(head->hash == 0){
			os << "_\n";
		}
		else {
			for(MapSlot<K, V>* slot = head; slot != nullptr; slot = slot->next){
				os << slot->key << " = " << slot->value << "(" << std::hex << slot->hash << ")   ";
			}
			os << "\n";
		}
	}
	return os;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, Option<T> v){
	if(!v.ok()){
		os << "Option(nil)";
	}
	else {
		os << "Option(" << v.unwrap_unchecked() << ")";
	}
	return os;
}

int main(){
	auto arena = Arena::make_virtual(10 * mem_MiB);
	auto allocator = arena.as_allocator();

	auto arr = DynamicArray<I32*>::make(allocator);
	defer(arr.destroy());
	I32 n = 0;
	print(sizeof(Option<U8>));
	print(sizeof(Option<U8*>));

	arr.append(&n);
	arr.append(&n);
	arr.append(&n);
	arr.append(&n);
	arr.append(nullptr);

	arr.remove_swap(0);

	print(arr.as_slice());
	// auto map = map_create<String, I32>(allocator, 8);
	// auto map2 = map_create<I32, String>(allocator, 8);
	// defer(destroy(&map));
	// defer(destroy(&map2));

	// print(map);
	// print(map2);
	// map_set(&map2, 69, "CU");

	// auto e = Option<I32>{1};
	// print(e);
	// e.clear();
	// print(e.or_else(69u));
	// e = 40;
	return 0;
}

