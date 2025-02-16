#include "base.hpp"
#include "debug_print.cpp"

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
				os << slot->key << " = " << slot->value << "(" << slot->hash << ")   ";
			}
			os << "\n";
		}
	}
	return os;
}

int main(){
	print("init");
	auto allocator = heap_allocator();

	auto map = map_create<String, I32>(allocator, 64);
	defer(destroy(&map));

	map_set(&map, "AAAA", 0);
	map_set(&map, "BBBB", 0);
	map_set(&map, "CCCC", 0);
	map_set(&map, "DDDD", 0);
	map_set(&map, "EEEE", 0);
	map_set(&map, "FFFF", 0);
	map_set(&map, "GGGG", 0);
	map_set(&map, "HHHH", 0);
	map_set(&map, "JJJJ", 0);
	map_set(&map, "KKKK", 0);
	map_set(&map, "IIII", 0);
	map_set(&map, "LLLL", 0);
	map_set(&map, "MMMM", 0);
	map_set(&map, "NNNN", 0);
	map_set(&map, "OOOO", 0);
	map_set(&map, "PPPP", 0);
	map_set(&map, "QQQQ", 0);
	map_set(&map, "RRRR", 0);
	map_set(&map, "SSSS", 0);
	map_set(&map, "TTTT", 0);
	map_set(&map, "WWWW", 0);
	map_set(&map, "XXXX", 0);
	map_set(&map, "YYYY", 0);
	map_set(&map, "ZZZZ", 0);

	map_set(&map, "lksajflkj", 0);
	map_set(&map, "lkasjflk", 0);
	map_set(&map, "9sdjajf", 0);
	map_set(&map, "askjflaj", 0);
	map_set(&map, "jalia", 0);
	map_set(&map, "pwqori2903i", 0);
	map_set(&map, "hakit", 0);
	map_set(&map, "jfaiwfh", 0);
	map_set(&map, "ycafjdliajw", 0);

	map_set(&map, "A", 0);
	map_set(&map, "B", 0);
	map_set(&map, "C", 0);
	map_set(&map, "D", 0);
	map_set(&map, "E", 0);
	map_set(&map, "F", 0);
	map_set(&map, "G", 0);
	map_set(&map, "H", 0);
	map_set(&map, "J", 0);
	map_set(&map, "K", 0);
	map_set(&map, "I", 0);
	map_set(&map, "L", 0);
	map_set(&map, "M", 0);
	map_set(&map, "N", 0);
	map_set(&map, "O", 0);
	map_set(&map, "P", 0);
	map_set(&map, "Q", 0);
	map_set(&map, "R", 0);
	map_set(&map, "S", 0);
	map_set(&map, "T", 0);
	map_set(&map, "W", 0);
	map_set(&map, "X", 0);
	map_set(&map, "Y", 0);
	map_set(&map, "Z", 0);

	map_set(&map, "a", 0);
	map_set(&map, "b", 0);
	map_set(&map, "c", 0);
	map_set(&map, "d", 0);
	map_set(&map, "e", 0);
	map_set(&map, "f", 0);
	map_set(&map, "g", 0);
	map_set(&map, "h", 0);
	map_set(&map, "j", 0);
	map_set(&map, "k", 0);
	map_set(&map, "i", 0);
	map_set(&map, "l", 0);
	map_set(&map, "m", 0);
	map_set(&map, "n", 0);
	map_set(&map, "o", 0);
	map_set(&map, "p", 0);
	map_set(&map, "q", 0);
	map_set(&map, "r", 0);
	map_set(&map, "s", 0);
	map_set(&map, "t", 0);
	map_set(&map, "w", 0);
	map_set(&map, "x", 0);
	map_set(&map, "y", 0);
	map_set(&map, "z", 0);

	map_set(&map, "0", 0);
	map_set(&map, "1", 0);
	map_set(&map, "2", 0);
	map_set(&map, "3", 0);
	map_set(&map, "4", 0);
	map_set(&map, "5", 0);
	map_set(&map, "6", 0);
	map_set(&map, "7", 0);
	map_set(&map, "8", 0);
	map_set(&map, "9", 0);

	print(map);

	return 0;
}

