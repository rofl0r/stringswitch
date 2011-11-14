typedef enum {
	stringswitch_enumerator_default_member_name(argv0),
	stringswitch_enumerator_member_name(argv0, hi),
	stringswitch_enumerator_member_name(argv0, lo),
	stringswitch_enumerator_member_name(argv0, bye),
	stringswitch_enumerator_member_name(argv0, hello),
	stringswitch_enumerator_member_name(argv0, holla),
	stringswitch_enumerator_member_name(argv0, hollu),
	stringswitch_enumerator_member_name(argv0, hodlu),
} stringswitch_enumerator_name(argv0);

static int stringswitch_enumerator_eval_func(argv0) (char* str, size_t l) {
	switch(l) {
		case 2:
			switch(str[0]) {
				case 'h':
					if(str[1]!='i') goto main_default;
					return stringswitch_enumerator_member_name(argv0, hi);
				case 'l':
					if(str[1]!='o') goto main_default;
					return stringswitch_enumerator_member_name(argv0, lo);
				default: goto main_default;
			}
		case 3:
			if(str[0]!='b') goto main_default;
			if(str[1]!='y') goto main_default;
			if(str[2]!='e') goto main_default;
			return stringswitch_enumerator_member_name(argv0, bye);
		case 5:
			if(str[0]!='h') goto main_default;
			switch(str[1]) {
				case 'e':
					if(str[2]!='l') goto main_default;
					if(str[3]!='l') goto main_default;
					if(str[4]!='o') goto main_default;
					return stringswitch_enumerator_member_name(argv0, hello);
				case 'o':
					switch(str[2]) {
						case 'l':
							if(str[3]!='l') goto main_default;
							switch(str[4]) {
								case 'a':
									return stringswitch_enumerator_member_name(argv0, holla);
								case 'u':
									return stringswitch_enumerator_member_name(argv0, hollu);
								default: goto main_default;
							}
						case 'd':
							if(str[3]!='l') goto main_default;
							if(str[4]!='u') goto main_default;
							return stringswitch_enumerator_member_name(argv0, hodlu);
						default: goto main_default;
					}
				default: goto main_default;
			}
		default:
			main_default:
			return stringswitch_enumerator_default_member_name(argv0);
	}
}
