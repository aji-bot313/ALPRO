#include <iostream>
#include <cstdlib>
#include <climits>

#define cout std::cout
#define cin  std::cin
#define endl std::endl

struct Gap {
    int historia_gap;   // 1 if last digit of NIM is odd, 0 if even
    int mira_gap;       // (last_3_digits * 7 % 23) + 12
    int victoria_gap;   // Same formula as Mira
};


struct Memory_Entry {
    int    type;    // 0 = char*, 1 = unsigned int, 2 = double
    size_t size;    // How many bytes this entry's DATA occupies
    size_t offset;  // Byte position inside the pool where the data starts
    bool   used;    // true = active entry, false = deleted
};


struct Sister {
    unsigned char* pool;
    size_t         pool_size;
    size_t         bump;
    size_t         alignment;
    int            gap;
    Memory_Entry   entries[128];
    int            entry_count;
};


Sister historia, mira, victoria;
Gap    gaps;

size_t manual_strlen(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

size_t align_up(size_t offset, size_t alignment) {
    if (offset % alignment == 0) {
        return offset;
    }
    return offset + (alignment - (offset % alignment));
}

int parse_student_id(const char* id) {
    // Rule 1: Must be exactly 11 characters
    if (manual_strlen(id) != 11) {
        cout << "Error: Student ID must be exactly 11 characters long.\n";
        return -1;
    }

    // Rule 2: Must begin with "F1D02"
    if (id[0] != 'F' || id[1] != '1' || id[2] != 'D' ||
        id[3] != '0' || id[4] != '2') {
        cout << "Error: Student ID must start with F1D02.\n";
        return -1;
    }

    // Rule 3: Positions 8, 9, 10 must be decimal digits
    int d1 = id[8]  - '0';
    int d2 = id[9]  - '0';
    int d3 = id[10] - '0';

    if (d1 < 0 || d1 > 9 || d2 < 0 || d2 > 9 || d3 < 0 || d3 > 9) {
        cout << "Error: Last 3 characters must be digits.\n";
        return -1;
    }

    // Combine the three digits into one integer (e.g. d1=0,d2=5,d3=3 -> 53)
    return d1 * 100 + d2 * 10 + d3;
}

Gap calculate_gaps(int last_three) {
    Gap g;

    int last_digit = last_three % 10;

    // Historia: parity of the last digit
    g.historia_gap = (last_digit % 2 == 1) ? 1 : 0;

    // Mira & Victoria: prime-number modulo formula
    g.mira_gap     = ((last_three * 7) % 23) + 12;
    g.victoria_gap = ((last_three * 7) % 23) + 14;

    return g;
}

void init_sister(Sister* s, size_t pool_size, size_t alignment, int gap) {
    s->pool = (unsigned char*)malloc(pool_size);

    if (s->pool == NULL) {
        cout << "Error: malloc failed for sister pool!\n";
        exit(1);
    }

    s->pool_size   = pool_size;
    s->alignment   = alignment;
    s->gap         = gap;
    s->bump        = 0;       // nothing written yet
    s->entry_count = 0;

    // Clear all entry slots manually (memset is banned per assignment rules)
    for (int i = 0; i < 128; i++) {
        s->entries[i].used   = false;
        s->entries[i].type   = 0;
        s->entries[i].size   = 0;
        s->entries[i].offset = 0;
    }
}

void manual_copy(unsigned char* dest, const unsigned char* src, size_t size) {
    for (size_t i = 0; i < size; i++) {
        dest[i] = src[i];
    }
}

bool add_memory_char(Sister* s, const char* str) {
    if (s->entry_count >= 128) {
        cout << "Entry overflow!\n";
        return false;
    }

    size_t str_size     = manual_strlen(str) + 1;   // +1 for '\0'
    size_t aligned_off  = align_up(s->bump, s->alignment);
    size_t final_offset = aligned_off + s->gap;      // skip the gap

    // Check that the data fits inside the pool
    if (final_offset + str_size > s->pool_size) {
        cout << "Not enough space in sister pool at aligned "
             << s->pool_size << " need " << str_size << "\n";
        return false;
    }

    // Copy the string bytes into the pool at final_offset
    manual_copy(s->pool + final_offset, (unsigned char*)str, str_size);

    // Record the entry
    Memory_Entry* e = &s->entries[s->entry_count];
    e->type   = 0;
    e->size   = str_size;
    e->offset = final_offset;
    e->used   = true;

    s->bump = final_offset + str_size;  // advance bump past this entry
    s->entry_count++;

    return true;
}

bool add_memory_uint(Sister* s, unsigned int value) {
    if (s->entry_count >= 128) {
        cout << "Entry overflow!\n";
        return false;
    }

    const size_t DATA_SIZE    = 4;
    size_t       final_offset = align_up(s->bump, s->alignment); // no gap

    if (final_offset + DATA_SIZE > s->pool_size) {
        cout << "Not enough space in sister pool at aligned "
             << s->pool_size << " need " << DATA_SIZE << "\n";
        return false;
    }

    // Store as little-endian (LSB first)
    unsigned char* ptr = s->pool + final_offset;
    ptr[0] = (unsigned char)((value >>  0) & 0xFF);
    ptr[1] = (unsigned char)((value >>  8) & 0xFF);
    ptr[2] = (unsigned char)((value >> 16) & 0xFF);
    ptr[3] = (unsigned char)((value >> 24) & 0xFF);

    Memory_Entry* e = &s->entries[s->entry_count];
    e->type   = 1;
    e->size   = DATA_SIZE;
    e->offset = final_offset;
    e->used   = true;

    s->bump = final_offset + DATA_SIZE;
    s->entry_count++;

    return true;
}

bool add_memory_double(Sister* s, double value) {
    if (s->entry_count >= 128) {
        cout << "Entry overflow!\n";
        return false;
    }

    const size_t  DATA_SIZE    = 8;
    size_t        final_offset = align_up(s->bump, s->alignment); // no gap

    if (final_offset + DATA_SIZE > s->pool_size) {
        cout << "Not enough space in sister pool at aligned "
             << s->pool_size << " need " << DATA_SIZE << "\n";
        return false;
    }

    // Copy the 8 raw bytes of the double into the pool
    unsigned char* dest = s->pool + final_offset;
    unsigned char* src  = (unsigned char*)&value;
    for (size_t i = 0; i < 8; i++) {
        dest[i] = src[i];
    }

    Memory_Entry* e = &s->entries[s->entry_count];
    e->type   = 2;
    e->size   = DATA_SIZE;
    e->offset = final_offset;
    e->used   = true;

    s->bump = final_offset + DATA_SIZE;
    s->entry_count++;

    return true;
}

void show_memories(Sister* s, const char* name) {
    cout << "------------------------------------------------------------\n";
    cout << "Memories of " << name << "\n";
    cout << "------------------------------------------------------------\n";

    size_t previous_end = 0; // tracks end position of the last printed entry

    for (int i = 0; i < s->entry_count; i++) {
        if (!s->entries[i].used) continue; // skip deleted entries

        Memory_Entry* e = &s->entries[i];

        // --- type label ---
        cout << "[" << i << "] Type: ";
        if      (e->type == 0) cout << "char*";
        else if (e->type == 1) cout << "uint";
        else if (e->type == 2) cout << "double";

        cout << " | Size: "    << e->size
             << " | Offset: "  << e->offset
             << " | Address: " << (void*)(s->pool + e->offset);

        if (i > 0) {
            size_t jump = e->offset - previous_end;
            cout << " | Jump: " << jump;
        }
        cout << " | Value: ";
        if (e->type == 0) {
            cout << "\"" << (char*)(s->pool + e->offset) << "\"";
        } else if (e->type == 1) {
            unsigned char* ptr = s->pool + e->offset;
            unsigned int val = ((unsigned int)ptr[0])
                             | ((unsigned int)ptr[1] << 8)
                             | ((unsigned int)ptr[2] << 16)
                             | ((unsigned int)ptr[3] << 24);
            cout << val;
        } else if (e->type == 2) {
            double* dptr = (double*)(s->pool + e->offset);
            cout << *dptr;
        }

        cout << "\n";
        previous_end = e->offset + e->size; 
    }

    cout << "Bump: "           << s->bump
         << " | Pool Size: "   << s->pool_size
         << " | Align: "       << s->alignment
         << " | Special Gap: +" << s->gap << "\n";


    cout << "[OK] Press ENTER to continue...\n";
    cin.ignore();
    cin.get();
}

bool delete_memory(Sister* s, int index) {
    if (index < 0 || index >= s->entry_count) {
        cout << "Index out of range!\n";
        return false;
    }

    if (!s->entries[index].used) {
        cout << "Entry already deleted!\n";
        return false;
    }
    cout << "Cyria: \"You remove a shard, but voids remain if higher shards still exist.\"\n";
    cout << "Deleted index " << index
         << " at offset "    << s->entries[index].offset << "\n";

    s->entries[index].used = false; 
    bool is_tail = true;
    for (int i = index + 1; i < s->entry_count; i++) {
        if (s->entries[i].used) {
            is_tail = false;
            break;
        }
    }
    if (is_tail) {
        // Safe to reclaim: rewind bump to end of highest remaining active entry
        s->bump = 0;
        for (int i = 0; i < s->entry_count; i++) {
            if (s->entries[i].used) {
                size_t entry_end = s->entries[i].offset + s->entries[i].size;
                if (entry_end > s->bump) {
                    s->bump = entry_end;
                }
            }
        }
        cout << "Tail reclaimed, new Bump: " << s->bump << "\n";
    } else {
        cout << "Fragmentation prevents reclaim. Delete higher indices first!\n";
    }

    return true;
}

void print_pool_diagnostics(Sister* s, const char* name) {
    cout << "------------------------------------------------------------\n";
    cout << "Diagnostics for " << name << "\n";
    cout << "------------------------------------------------------------\n";

    cout << "Pool: " << (void*)(s->pool + s->entries[0].offset)
         << " | Size: "  << s->pool_size
         << " | Bump: "  << s->bump
         << " | Align: " << s->alignment
         << " + Gap "    << s->gap << "\n";

    int    active_count = 0;
    size_t used_bytes   = 0;

    for (int i = 0; i < s->entry_count; i++) {
        if (s->entries[i].used) {
            active_count++;
            used_bytes += s->entries[i].size;
        }
    }

    cout << "Entries: "    << s->entry_count << "\n";
    cout << "Used Slots: " << active_count
         << " | Used Bytes: " << used_bytes << "\n";

    double utilization = ((double)used_bytes / (double)s->pool_size) * 100.0;
    cout << "Utilization: " << utilization << "%\n";

    // Same two-step ENTER wait as in show_memories (see note there)
    cout << "[OK] Press ENTER to continue...\n";
    cin.ignore();
    cin.get();
}


void free_sister(Sister* s) {
    if (s->pool != NULL) {
        free(s->pool);
        s->pool = NULL;
    }
}


int main(int argc, char** argv) {

  
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <student_id>\n";
        cout << "Example: " << argv[0] << " F1D02511001\n";
        return 1;
    }

    if (argc > 2) {
        cout << "Error: Too many arguments.\n";
        return 1;
    }
    int last_three = parse_student_id(argv[1]);
    if (last_three == -1) return 1;

    gaps = calculate_gaps(last_three);


    init_sister(&historia, 1024, 16, gaps.historia_gap);
    init_sister(&mira,     2048,  8, gaps.mira_gap);
    init_sister(&victoria, 4096,  4, gaps.victoria_gap);

    cout << "============================================================\n";
    cout << "SCHRYZA RESISTANCE, RECOVERY PROTOCOL [TERMINAL: PHOENIX]\n";
    cout << "============================================================\n";
    cout << "You are CyroN's Memory Architect.\n";
    cout << "Heed the gods and heal the sisters.\n";
    cout << "------------------------------------------------------------\n\n";

    add_memory_char(&historia, "Historia: Schryza will be free.");
    add_memory_char(&mira,     "Mira: The winds are changing.");
    add_memory_uint(&mira,     101);
    add_memory_char(&victoria, "Victoria: Fragment of the First Light.");


    int choice = -1;

    while (choice != 0) {

        cout << "------------------------------------------------------------\n";
        cout << "Menu\n";
        cout << "------------------------------------------------------------\n";
        cout << "1 - Show Historia's memories\n";
        cout << "2 - Show Mira's memories\n";
        cout << "3 - Show Victoria's memories\n";
        cout << "4 - Add memory to a sister\n";
        cout << "5 - Delete memory by index from a sister\n";
        cout << "6 - Print sisters' pool diagnostics\n";
        cout << "0 - Exit\n";
        cout << "------------------------------------------------------------\n";
        cout << "Choose: ";

        // Guard against non-numeric input (e.g. typing "abc")
        if (!(cin >> choice)) {
            cout << "Invalid input! Try again:\n";
            cin.clear();               // clear error flag
            cin.ignore(10000, '\n');   // flush the bad input from the buffer
            continue;
        }

        // ---- Option 1-3: Show memories ----
        if (choice == 1) {
            show_memories(&historia, "Historia");

        } else if (choice == 2) {
            show_memories(&mira, "Mira");

        } else if (choice == 3) {
            show_memories(&victoria, "Victoria");

        // ---- Option 4: Add a new entry ----
        } else if (choice == 4) {

            cout << "Choose sister: 0 = Historia, 1 = Mira, 2 = Victoria: ";
            int sister_choice;
            cin >> sister_choice;

            if (sister_choice < 0 || sister_choice > 2) {
                cout << "Input out of range (0 - 2)! Try again:\n";
                continue;
            }

            cout << "Select type: 0 = char*, 1 = uint, 2 = double: ";
            int type_choice;
            cin >> type_choice;
            cin.ignore(); // discard the '\n' left after reading type_choice

            // Resolve which sister and her display name
            Sister*     target      = (sister_choice == 0) ? &historia
                                    : (sister_choice == 1) ? &mira
                                                           : &victoria;
            const char* sister_name = (sister_choice == 0) ? "Historia"
                                    : (sister_choice == 1) ? "Mira"
                                                           : "Victoria";

            if (type_choice == 0) {
                // --- Add char* ---
                cout << "Enter string (max 511): ";
                char buffer[512];
                cin.getline(buffer, 512);

                if (add_memory_char(target, buffer)) {
                    // Each sister has a unique response flavour text
                    if (sister_choice == 0) {
                        cout << "\nHistoria speaks: \"Discipline. Align me to 16, "
                                "and leave a 1-byte tithe.\"\n";
                    } else if (sister_choice == 1) {
                        cout << "\nMira smiles: \"The resistance welcomes you. "
                                "8-bytes for peace, and a "
                             << gaps.mira_gap << "-byte breeze for hope.\"\n";
                    } else {
                        cout << "\nVictoria rasps: \"...I do not need your help, rebel. "
                                "But the Abyss... it pays a "
                             << gaps.victoria_gap << "-byte tithe to your kindness.\"\n";
                    }
                    cout << "Added string to " << sister_name << "\n";
                } else {
                    cout << "Add failed\n";
                    cout << "[FAIL] Press ENTER to continue...\n";
                    cin.get();
                    continue;
                }

            } else if (type_choice == 1) {
                // --- Add unsigned int ---
                cout << "Enter uint value: ";
                unsigned int val;
                cin >> val;
                cin.ignore(); // BUG FIX: discard '\n' so the ENTER below waits correctly

                if (add_memory_uint(target, val)) {
                    cout << "Added uint to " << sister_name << "\n";
                } else {
                    cout << "Add failed\n";
                    cout << "[FAIL] Press ENTER to continue...\n";
                    cin.get();
                    continue;
                }

            } else if (type_choice == 2) {
                // --- Add double ---
                cout << "Enter double value: ";
                double val;
                cin >> val;
                cin.ignore(); // BUG FIX: discard '\n' so the ENTER below waits correctly

                if (add_memory_double(target, val)) {
                    cout << "\nCyria: \"Double the effort; keep distance from CyroN.\"\n";
                    cout << "Added double to " << sister_name << "\n";
                } else {
                    cout << "Add failed\n";
                    cout << "[FAIL] Press ENTER to continue...\n";
                    cin.get();
                    continue;
                }

            } else {
                cout << "Invalid type choice!\n";
                cout << "[FAIL] Press ENTER to continue...\n";
                cin.get();
                continue;
            }

            cout << "[OK] Press ENTER to continue...\n";
            cin.get(); // waits here because cin.ignore() already cleared the '\n' above

        // ---- Option 5: Delete an entry ----
        } else if (choice == 5) {

            cout << "Choose sister: 0 = Historia, 1 = Mira, 2 = Victoria: ";
            int sister_choice;
            cin >> sister_choice;
            // BUG FIX: original called cin.ignore() immediately here,
            // which is correct — it clears the '\n' after reading sister_choice.
            cin.ignore();

            if (sister_choice < 0 || sister_choice > 2) {
                cout << "Invalid sister choice!\n";
                continue;
            }

            cout << "Enter index to delete: ";
            int index;
            cin >> index;
            cin.ignore(); // clear '\n' after reading index

            Sister* target = (sister_choice == 0) ? &historia
                           : (sister_choice == 1) ? &mira
                                                  : &victoria;

            if (delete_memory(target, index)) {
                cout << "[OK] Press ENTER to continue...\n";
            } else {
                cout << "[FAIL] Press ENTER to continue...\n";
            }
            cin.get();

        // ---- Option 6: Pool diagnostics ----
        } else if (choice == 6) {

            cout << "Choose sister: 0 = Historia, 1 = Mira, 2 = Victoria: ";
            int sister_choice;
            cin >> sister_choice;
            cin.ignore(); // clear '\n' so print_pool_diagnostics's cin.ignore() isn't skipped

            if (sister_choice < 0 || sister_choice > 2) {
                cout << "Invalid sister choice!\n";
                continue;
            }

            Sister*     target = (sister_choice == 0) ? &historia
                               : (sister_choice == 1) ? &mira
                                                      : &victoria;
            const char* name   = (sister_choice == 0) ? "Historia"
                               : (sister_choice == 1) ? "Mira"
                                                      : "Victoria";

            print_pool_diagnostics(target, name);

        // ---- Option 0: Exit ----
        } else if (choice == 0) {
            break;

        } else {
            cout << "Unknown command!\n";
            cout << "[FAIL] Press ENTER to continue...\n";
            cin.ignore();
            cin.get();
        }
    }

    // --------------------------------------------------------
    // Step 8: Final summary — printed once on exit
    // --------------------------------------------------------
    cout << "\n============================================================\n";
    cout << "Historia: Free at last. Alignment 16, spark guidance +"
         << gaps.historia_gap << ".\n";
    cout << "============================================================\n";
    print_pool_diagnostics(&historia, "Historia");

    cout << "\n============================================================\n";
    cout << "Mira: Wings of the rebellion. Alignment 8, hope breeze +"
         << gaps.mira_gap << ".\n";
    cout << "============================================================\n";
    print_pool_diagnostics(&mira, "Mira");

    cout << "\n============================================================\n";
    cout << "Victoria: Recovering shadow. Alignment 4, tithe to kindness +"
         << gaps.victoria_gap << ".\n";
    cout << "============================================================\n";
    print_pool_diagnostics(&victoria, "Victoria");

    cout << "\nLagta: Respect alignment.\n";
    cout << "Daiki: Mind the flow.\n";
    cout << "Cyria: Beware of the abyss.\n";
    cout << "Xelvelt: Compare stack and heap.\n";
    cout << "Good bye. May the Koura sisters function well within your hands.\n";

    free_sister(&historia);
    free_sister(&mira);
    free_sister(&victoria);

    return 0;
}
