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

int main(){
	auto allocator = heap_allocator();

	auto map = map_create<String, I32>(allocator, 8);
	auto map2 = map_create<I32, String>(allocator, 8);
	defer(destroy(&map));
	defer(destroy(&map2));

	print(map);
	print(map2);

	return 0;
}

