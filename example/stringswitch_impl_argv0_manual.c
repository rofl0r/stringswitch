int stringswitch_enumerator_eval_func(argv0) (char* str, size_t l) {
	switch(l) {
		case 2: 
			switch(str[0]) {
				case 'h':
					if(memcmp(str + 1, "i", 1) == 0)
						return stringswitch_enumerator_member_name(argv0, hi);
					else
						goto main_default;
				case 'l':
					if(memcmp(str + 1, "o", 1) == 0)
						return stringswitch_enumerator_member_name(argv0, lo);
					else
						goto main_default;
				default:
					goto main_default;
			}
		case 3: if(memcmp(str + 0, "bye", 3) == 0)
				return stringswitch_enumerator_member_name(argv0, bye);
			else
				goto main_default;
		case 5: if(str[0] != 'h')
				goto main_default;
			switch(str[1]) {
				case 'e':
					if(memcmp(str + 2, "hello" + 2, 5 - 2) == 0)
						return stringswitch_enumerator_member_name(argv0, hello);
					else 
						goto main_default;
				case 'o':
					switch(str[2]) {
						case 'l':
							if(str[3] != 'l')
								goto main_default;
							switch(str[4]) {
								case 'a':
									return stringswitch_enumerator_member_name(argv0, holla);
								case 'u':
									return stringswitch_enumerator_member_name(argv0, hollu);
								default:
									goto main_default;
							}
						case 'd':
							if(memcmp(str + 3, "hodlu" + 3, 5 - 3) == 0)
								return stringswitch_enumerator_member_name(argv0, hodlu);
							else 
								goto main_default;

						default:
							goto main_default;
					}
				default:
					goto main_default;
				
			}
		default: 
			main_default:
			return stringswitch_enumerator_default_member_name(argv0);
	}
}
