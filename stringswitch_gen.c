/*
    Copyright (C) 2011  rofl0r

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include "../lib/include/strlib.h"
#include "../lib/include/stringptrlist.h"
#include "../lib/include/stringptr.h"
#include "../lib/include/hashlist.h"
#include "../lib/include/fileparser.h"
#include "../lib/include/logger.h"
#include "../lib/include/strswitch.h"
#include <fcntl.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>

typedef sblist size_t_ptrlist;

typedef struct {
	size_t len;
	stringptrlist* list;
} hashrec;

typedef struct {
	stringptr* id;
	uint32_t hash;
	hashlist* h;
} sectionrec;

static uint32_t daliashash_sp(stringptr* str) {
	uint_fast32_t h = 0;
	char* s = str->ptr;
	size_t size = str->size;
	while (size) {
		h = 16*h + *s++;
		h ^= h>>24 & 0xf0;
		size--;
	}
	return h & 0xfffffff;
}

stringptr normalize(stringptr* id, char* workbuf, ssize_t buflen) {
	size_t i;
	size_t os = buflen;
	char hexbuf[5];
	stringptr res = {workbuf, 0};
	
	for(i = 0; i < id->size; i++) {
		switch(id->ptr[i]) {
			case STRSWITCH_ALPHA:
				buflen--;
				if(buflen <= 0) goto err;
				*workbuf++ = id->ptr[i];
				break;
			default:
				buflen -= 4;
				if(buflen <= 0) goto err;
				ulz_snprintf(hexbuf, 5, "0x%.2X", id->ptr[i]);
				memcpy(workbuf, hexbuf, 4);
				workbuf += 4;
				break;
		}
		
	}
	if(buflen > 0)
		*workbuf = 0;
	
	res.size = os - buflen;
	
	out:
	return res;
	err:
	res.ptr = NULL;
	goto out;
}

typedef sblist variant_info_list;

typedef struct {
	int c;
	size_t_ptrlist* members;
	variant_info_list* variants;
} variant_info;


variant_info* find_variant(sblist* variants, int c) {
	size_t i;
	variant_info* res;
	for(i = 0; i < sblist_getsize(variants); i++) {
		res = sblist_get(variants, i);
		if(res->c == c) return res;
	}
	return NULL;
}

int in_member_list(variant_info* variant, size_t index) {
	size_t i, *r;
	for(i = 0; i < sblist_getsize(variant->members); i++) {
		r = sblist_get(variant->members, i);
		if (*r == index) return 1;
	}
	return 0;
}

void write_enum(int outfd, sectionrec* sref) {
	hashlist* h;
	hashlist_iterator iter2;
	hashrec *href;
	stringptrlist* newlist;
	size_t i;
	stringptr *entry, normalized;
	char norm_buf[1024];
	char buf[1024];
	
	log_put(outfd, VARISL("typedef enum {\n\tstringswitch_enumerator_default_member_name("), VARIS(sref->id), VARISL("),"), NULL);
	
	h = sref->h;
	hashlist_iterator_init(&iter2);
	while((href = hashlist_next(h, &iter2))) {
		newlist = href->list;
		for(i = 0; i < stringptrlist_getsize(newlist); i++) {
			entry = stringptrlist_get(newlist, i);
			log_put(1, VARISL("id "), VARIS(sref->id), VARISL(", entry: "), VARII((int) entry->size), VARISL(", "), VARIS(entry), NULL);
			normalized = normalize(entry, norm_buf, sizeof(norm_buf));
			ulz_snprintf(buf, sizeof(buf), "\tstringswitch_enumerator_member_name(%s, %s),\n", sref->id->ptr, normalized.ptr);
			log_putc(outfd, buf);
		}
	}
	
	log_put(outfd, VARISL("} stringswitch_enumerator_name("), VARIS(sref->id), VARISL(");\n"), NULL);
}


// initialise with members list, containing all ids
variant_info_list* get_variants(size_t_ptrlist* members, stringptr* last_prefix, stringptrlist* block_strings, size_t block_len) {
	size_t i;
	variant_info vr, *vp;
	stringptr* next_prefix, *act;
	size_t* id;
	if(last_prefix->size == block_len) return NULL;
	variant_info_list* res = NULL;
	stringptr temp;
	char c;
	
	for(i = 0; i < sblist_getsize(members); i++)  {
		id = sblist_get(members, i);
		act = stringptrlist_get(block_strings, *id);
		if(last_prefix->size == 0 || stringptr_here(act, 0, last_prefix)) {
			if(!res) res = sblist_new(sizeof(variant_info), 16);
			if(!(vp = find_variant(res, act->ptr[last_prefix->size]))) {
				vr.c = act->ptr[last_prefix->size];
				vr.members = sblist_new(sizeof(size_t), stringptrlist_getsize(members));
				vr.variants = NULL;
				sblist_add(res, &vr);
				vp = find_variant(res, vr.c);
			}
			sblist_add(vp->members, id);
		}
	}
	if(res) {
		for (i = 0; i < sblist_getsize(res); i++) {
			vp = sblist_get(res, i);
			c = vp->c;
			temp.size = 1;
			temp.ptr = &c;
			next_prefix = stringptr_concat(last_prefix, &temp, NULL);
			vp->variants = get_variants(vp->members, next_prefix, block_strings, block_len);
		}
	}
	return res;
}

void print_level_tabs(int fd, size_t level) {
	size_t i;
	for (i = 0; i < level + 3; i++)
		ulz_fprintf(fd, "\t");
}

void dump_variants(int outfd, stringptr* section_name, variant_info_list* variants, 
		   stringptrlist* block_strings, size_t level, size_t previous_membercount, ssize_t backindent) {
	if(variants == NULL) return;
	variant_info* vp;
	stringptr* act, normalized;
	char norm_buf[1024];
	size_t membercount;
	size_t* id;
	int switched = 0;
	sblist_iter(variants, vp) {
		membercount = sblist_getsize(vp->members);
		if(membercount == previous_membercount) {
			print_level_tabs(outfd, level - backindent);
			ulz_fprintf(outfd, "if(str[%zu]!='%c') goto main_default;\n", level, vp->c);
			backindent++;
		} else {
			if(!switched) {
				print_level_tabs(outfd, level - backindent);
				ulz_fprintf(outfd, "switch(str[%zu]) {\n", level);
				switched = 1;
				backindent--;
			}
			print_level_tabs(outfd, level - backindent);
			ulz_fprintf(outfd, "case '%c':\n", vp->c);
		}
		if(!vp->variants) {
			print_level_tabs(outfd, level - backindent);
			id = sblist_get(vp->members, 0);
			act = sblist_get(block_strings, *id);
			normalized = normalize(act, norm_buf, sizeof(norm_buf));
			log_put(outfd, VARISL("\treturn stringswitch_enumerator_member_name("), 
				VARIS(section_name), VARISL(", "), VARIS(&normalized), VARISL(");"), NULL);
		}
		dump_variants(outfd, section_name, vp->variants, block_strings, level+1, membercount, backindent);
	}
	if(switched) {
		print_level_tabs(outfd, level - backindent);
		ulz_fprintf(outfd, "default: goto main_default;\n");
		backindent++;
		print_level_tabs(outfd, level - backindent);
		ulz_fprintf(outfd, "}\n");
	}
}

void free_variants(variant_info_list* v) {
	(void) v;
	// TODO
}

void write_length_block(int outfd, hashrec* block_href, stringptr* section_name) {
	stringptrlist* newlist;
	size_t i;
	stringptr temp;
	
			
	newlist = block_href->list;
	log_put(outfd, VARISL("\t\tcase "),VARII((int) block_href->len), VARISL(":"), NULL);
	
	size_t_ptrlist* initial_members = sblist_new(sizeof(size_t), stringptrlist_getsize(block_href->list));
	temp.size = 0;
	temp.ptr = NULL;
	
	for(i = 0; i < stringptrlist_getsize(block_href->list); i++)
		sblist_add(initial_members, &i);
	
	variant_info_list* variants = get_variants(initial_members, &temp, block_href->list, block_href->len);
	dump_variants(outfd, section_name, variants, block_href->list, 0, stringptrlist_getsize(block_href->list), 0);
	
	sblist_free(initial_members);
	free_variants(variants);
	
}

void codegen(hashlist* hashlists) {
	sectionrec *sref;
	hashrec *href;

	hashlist_iterator iter, iter2;
	hashlist_iterator_init(&iter);
	int outfd = -1;
	char buf[1024];
	
	if(hashlists) while((sref = hashlist_next(hashlists, &iter))) {
		
		// iterating through the "sections", that is each different strswitch usage
		ulz_snprintf(buf, sizeof(buf), "stringswitch_impl_%s.c", sref->id->ptr);

		outfd = open(buf, O_CREAT | O_TRUNC | O_WRONLY, 0660);
		if(outfd == -1) {
			log_putc(2, buf);
			log_perror(" : couldnt open");
			return;
		}
		log_put(1, VARISL("writing to "), VARIC(buf), NULL);
		write_enum(outfd, sref);
		
		log_put(outfd, VARISL("static int stringswitch_enumerator_eval_func("), VARIS(sref->id), 
			VARISL(") (char* str, size_t l) {"), NULL);
		log_put(outfd, VARISL("\tswitch(l) {"), NULL);

		
		hashlist_iterator_init(&iter2);
		while((href = hashlist_next(sref->h, &iter2))) {
			// iterating through the different length blocks in our switch statement
			
			write_length_block(outfd, href, sref->id);
			
		}
		
		log_put(outfd, VARISL("\t\tdefault:\n\t\t\tmain_default:\n\t\t\treturn stringswitch_enumerator_default_member_name("),
			VARIS(sref->id), VARISL(");\n\t}\n}"), NULL);
		
		close(outfd);
	}
}


void free_sections_hashlist(hashlist* l) {
	// TODO ... free recursive
	(void) l;
}

// we have a hashlist, "hashlists", in which we will insert one record of type
// sectionrec per different "section". a section is the identifier of a stringswitch statement.
// this rec contains a reference to another hashlist, which will contain one stringptrlist per 
// length of the "add"ed entries.

static stringptr* command_prefix = SPL("//stringswitch_gen ");
hashlist* parse(char* file) {
	hashlist *hashlists = NULL;
	hashlist *h = NULL;
	fileparser fb, *f = &fb;
	stringptr line;
	stringptr command;
	stringptr param1, param2;
	size_t x;
	hashrec hr, *href;
	sectionrec sr, *sref;
	sblist* sbt;
	stringptrlist* newlist;
	uint32_t p1hash;
	size_t i;
	
	fileparser_open(f, file);
	while(!fileparser_readline(f) && !fileparser_getline(f, &line)) {
		if(stringptr_here(&line, 0, command_prefix)) {
			
			command.ptr = line.ptr + command_prefix->size;
			x = 0;
			while(command_prefix->size + x  < line.size && 
			      !(command.ptr[x] == ' ' || command.ptr[x] == 0)) x++;
			command.size = x;
			
			//log_puts(1, &command); log_putln(1);
			
			param1.ptr = command.ptr + x + 1;
			if(param1.ptr >= line.ptr + line.size) {
				ulz_printf("no param1\n");
				goto err;
			}
			x = 0;
			while(command_prefix->size + command.size + x + 1  < line.size &&
			      !(param1.ptr[x] == ' ' || param1.ptr[x] == 0)) 
				x++;
			param1.size = x;
			
			//log_puts(1, &param1); log_putln(1);
			
			if(!hashlists)
				hashlists = hashlist_new(64, sizeof(sectionrec));
			
			p1hash = daliashash_sp(&param1);
			
			h = NULL;
			sbt = hashlist_get(hashlists, p1hash);
			if(!sbt) {
				insert_hashlist:
				h = hashlist_new(64, sizeof(hashrec));
				sr.h = h;
				sr.hash = p1hash;
				sr.id = stringptr_copy(&param1);
				hashlist_add(hashlists, p1hash, &sr);
				sbt = hashlist_get(hashlists, p1hash);
			}
			if(!h)
				for(i = 0; i < sblist_getsize(sbt); i++) {
					sref = sblist_get(sbt, i);
					if(sref->hash == p1hash && EQ(&param1, sref->id)) {
						h = sref->h;
						break;
					}
				}
			
			if(!h) goto insert_hashlist;
			
			
			if(EQ(&command, SPL("add"))) {
				
				param2.ptr = param1.ptr + x + 1;

				if(param2.ptr >= line.ptr + line.size) {
					ulz_printf("no param2\n");
					goto err;
				}
				
				if(param2.ptr[0] != '"') {
					ulz_printf("add parameter must be double-quoted\n");
					goto err;
				}
				param2.ptr++;
				
				if(param2.ptr >= line.ptr + line.size) {
					ulz_printf("no param2\n");
					goto err;
				}
				
				x = 0;
				while(command_prefix->size + command.size + param1.size + x + 2 + 1 < line.size && 
				      !(param2.ptr[x] == '"' || param2.ptr[x] == 0)) x++;
				param2.size = x;
				
				//log_puts(1, &param2); log_putln(1);
				
				newlist = NULL;
				sbt = hashlist_get(h, param2.size);
				if(!sbt) {
					insert_stringptrlist:
					newlist = stringptrlist_new(4);
					hr.len = param2.size;
					hr.list = newlist;
					hashlist_add(h, param2.size, &hr);
					sbt = hashlist_get(h, param2.size);
				}
				if(!newlist)
					for(i = 0; i < sblist_getsize(sbt); i++) {
						href = sblist_get(sbt, i);
						if(href->len == param2.size) {
							newlist = href->list;
							break;
						}
					}
				if(!newlist) goto insert_stringptrlist;	
				stringptrlist_add(newlist, stringptr_strdup(&param2), param2.size);
			} else {
				#if 0
				param2.size = 0;
				param2.ptr = 0;
				#else
				log_put(1, VARISL("the \"add\" command needs an additional parameter embedded in double quotes!"), NULL);
				goto err;
				#endif
			}
		}
	}
	
	goto out;
	err:
	if(hashlists) free_sections_hashlist(hashlists);
	hashlists = NULL;
	out:
	fileparser_close(f);
	
	return hashlists;
}

int main(int argc, char** argv) {
	int i;
	hashlist* sections;
	ulz_printf("stringswitch generator 1.0 by rofl0r\n");
	ulz_printf("^~^~^~^~^~^~^~^~^~^~^~^~^~^~^~^~^~^~\n");
	if(argc == 1) {
		ulz_printf("pass one or more filenames containing stringswitch directives embedded as C99 comments!\n");
		return 1;
	}
	for(i = 1; i < argc; i++) {
		if(access(argv[i], R_OK) != -1) {
			sections = parse(argv[i]);
			if(!sections) return 1;
			codegen(sections);
		}
	}
	return 0;
}


