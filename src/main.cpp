#include "base.hpp"
#include "debug_print.cpp"

template<typename K, typename V>
std::ostream& operator<<(std::ostream& os, Map<K, V> map){
	// for(Size i = 0; i < map.capacity; i ++){
	// 	auto head = &map.base_slots[i];
	//
	// 	os << "[" << i << "]\t";
	// 	if(head->hash == 0){
	// 		os << "_\n";
	// 	}
	// 	else {
	// 		for(MapSlot<K, V>* slot = head; slot != nullptr; slot = slot->next){
	// 			os << slot->key << " = " << slot->value << "(" << slot->hash << ")   ";
	// 		}
	// 		os << "\n";
	// 	}
	// }

	os << "bucket,count\n";
	for(Size i = 0; i < map.capacity; i ++){
		auto head = &map.base_slots[i];

		os << i << ",";
		if(head->hash == 0){
			os << "0\n";
		}
		else {
			Size count = 0;
			for(MapSlot<K, V>* slot = head; slot != nullptr; slot = slot->next){
				count += 1;
			}
			os << count << "\n";
		}
	}
	return os;
}

int main(){
	auto allocator = heap_allocator();

	auto map = map_create<String, I32>(allocator, 1024);
	defer(destroy(&map));

	print(map);

	return 0;
}

