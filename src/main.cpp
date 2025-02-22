#include "base.hpp"
// #include "debug_print.cpp"

// template<typename K, typename V>
// std::ostream& operator<<(std::ostream& os, Map<K, V> map){
// 	for(isize i = 0; i < map.capacity; i ++){
// 		auto head = &map.base_slots[i];
//
// 		os << "[" << i << "]\t";
// 		if(head->hash == 0){
// 			os << "_\n";
// 		}
// 		else {
// 			for(MapSlot<K, V>* slot = head; slot != nullptr; slot = slot->next){
// 				os << slot->key << " = " << slot->value << "(" << std::hex << slot->hash << ")   ";
// 			}
// 			os << "\n";
// 		}
// 	}
// 	return os;
// }

template<typename T, typename E>
std::ostream& operator<<(std::ostream& os, Result<T, E> v){
	if(!v.ok()){
		os << "Error(" << v.error << ")";
	}
	else {
		os << "Value(" << v.value << ")";
	}
	return os;
}

int main(){
	auto arena = Arena::make_virtual(10 * mem_MiB);
	auto allocator = arena.as_allocator();

	auto [arr, err] = DynamicArray<i32*>::make(allocator);
	ensure(ok(err), "Failed to create dyn arr");
	defer(arr.destroy());

	i32 n = 0;
	print(sizeof(Result<u32, String>));

	arr.append(&n);
	arr.append(&n);
	arr.append(&n);
	arr.append(&n);

	arr.append(nullptr);

	arr.remove_swap(0);

	print(arr.as_slice());
	// auto map = map_create<String, i32>(allocator, 8);
	// auto map2 = map_create<i32, String>(allocator, 8);
	// defer(destroy(&map));
	// defer(destroy(&map2));

	// print(map);
	// print(map2);
	// map_set(&map2, 69, "CU");

	// auto e = Option<i32>{1};
	// print(e);
	// e.clear();
	// print(e.or_else(69u));
	// e = 40;
	return 0;
}

