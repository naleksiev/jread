/*
 * Copyright 2016 Nikolay Aleksiev. All rights reserved.
 * License: https://github.com/naleksiev/jread/blob/master/LICENSE
 */

#include "jread.h"

#include <stdio.h>
#include <stdlib.h>

#define JR_DISPATCH_NEXT()  goto *go[*(c = cstr++)]
#define JR_DISPATCH_THIS()  goto *go[*c];
#define JR_PUSH(x)          go_stack[go_stack_idx++] = go
#define JR_PUSH_GO(x)       go_stack[go_stack_idx++] = go; go = x
#define JR_POP_GO()         go = go_stack[--go_stack_idx]

void jr_read(jr_callback cb, const char* cstr, void* user_data) {
    static void* go_doc[] = {
        ['\0']        = &&l_done,
        [1 ... 8]     = &&l_err,
        ['\t']        = &&l_next,
        ['\n']        = &&l_next,
        [11 ... 12]   = &&l_err,
        ['\r']        = &&l_next,
        [14 ... 31]   = &&l_err,
        [' ']         = &&l_next,
        [33 ... 33]   = &&l_err,
        ['"']         = &&l_str_s,
        [35 ... 44]   = &&l_err,
        ['-']         = &&l_num_s,
        [46 ... 47]   = &&l_err,
        ['0' ... '9'] = &&l_num_s,
        [58 ... 90]   = &&l_err,
        ['[']         = &&l_arr_s,
        [92 ... 101]  = &&l_err,
        ['f']         = &&l_false_f,
        [103 ... 109] = &&l_err,
        ['n']         = &&l_null_n,
        [111 ... 115] = &&l_err,
        ['t']         = &&l_true_t,
        [117 ... 122] = &&l_err,
        ['{']         = &&l_obj_s,
        [124 ... 255] = &&l_err,
    };

    static void* go_val[] = {
        [0 ... 8]     = &&l_err,
        ['\t']        = &&l_next,
        ['\n']        = &&l_next,
        [11 ... 12]   = &&l_err,
        ['\r']        = &&l_next,
        [14 ... 31]   = &&l_err,
        [' ']         = &&l_next,
        [33 ... 33]   = &&l_err,
        ['"']         = &&l_str_s,
        [35 ... 44]   = &&l_err,
        ['-']         = &&l_num_s,
        [46 ... 47]   = &&l_err,
        ['0' ... '9'] = &&l_num_s,
        [58 ... 90]   = &&l_err,
        ['[']         = &&l_arr_s,
        [92 ... 101]  = &&l_err,
        ['f']         = &&l_false_f,
        [103 ... 109] = &&l_err,
        ['n']         = &&l_null_n,
        [111 ... 115] = &&l_err,
        ['t']         = &&l_true_t,
        [117 ... 122] = &&l_err,
        ['{']         = &&l_obj_s,
        [124 ... 255] = &&l_err,
    };

    static void* go_num[] = {
        [0 ... 45]    = &&l_num_e,
        ['.']         = &&l_next,
        [47 ... 47]   = &&l_num_e,
        ['0' ... '9'] = &&l_next,
        [58 ... 255]  = &&l_num_e,
    };

    static void* go_str[] = {
        [0 ... 31]    = &&l_err,
        [32 ... 33]   = &&l_next,
        ['"']         = &&l_str_e,
        [35 ... 126]  = &&l_next,
        [127 ... 191] = &&l_err,
        [192 ... 223] = &&l_utf8_2,
        [224 ... 239] = &&l_utf8_3,
        [240 ... 247] = &&l_utf8_4,
        [248 ... 255] = &&l_err,
    };

    static void* go_key[] = {
        [0 ... 31]    = &&l_err,
        [32 ... 33]   = &&l_next,
        ['"']         = &&l_key,
        [35 ... 126]  = &&l_next,
        [127 ... 191] = &&l_err,
        [192 ... 223] = &&l_utf8_2,
        [224 ... 239] = &&l_utf8_3,
        [240 ... 247] = &&l_utf8_4,
        [248 ... 255] = &&l_err,
    };

    static void* go_utf8[] = {
        ['\0']        = &&l_utf8_valid,
        [1 ... 127]   = &&l_err,
        [128 ... 191] = &&l_utf8,
        [192 ... 255] = &&l_err,
    };

    static void* go_utf8_valid[] = {
        ['\0']        = &&l_next,
        [1  ... 255]  = &&l_err,
    };

    static void* go_null_n[] = {
        [0 ... 116]   = &&l_err,
        ['u']         = &&l_null_u,
        [118 ... 255] = &&l_err,
    };

    static void* go_null_u[] = {
        [0 ... 107]   = &&l_err,
        ['l']         = &&l_null_l,
        [109 ... 255] = &&l_err,
    };

    static void* go_null_l[] = {
        [0 ... 107]   = &&l_err,
        ['l']         = &&l_null_ll,
        [109 ... 255] = &&l_err,
    };

    static void* go_true_t[] = {
        [0 ... 113]   = &&l_err,
        ['r']         = &&l_true_r,
        [115 ... 255] = &&l_err,
    };

    static void* go_true_r[] = {
        [0 ... 116]   = &&l_err,
        ['u']         = &&l_true_u,
        [118 ... 255] = &&l_err,
    };

    static void* go_true_u[] = {
        [0 ... 100]   = &&l_err,
        ['e']         = &&l_true_e,
        [102 ... 255] = &&l_err,
    };

    static void* go_false_f[] = {
        [0 ... 96]    = &&l_err,
        ['a']         = &&l_false_a,
        [98 ... 255]  = &&l_err,
    };

    static void* go_false_a[] = {
        [0 ... 107]   = &&l_err,
        ['l']         = &&l_false_l,
        [109 ... 255] = &&l_err,
    };

    static void* go_false_l[] = {
        [0 ... 114]   = &&l_err,
        ['s']         = &&l_false_s,
        [116 ... 255] = &&l_err,
    };

    static void* go_false_s[] = {
        [0 ... 100]   = &&l_err,
        ['e']         = &&l_false_e,
        [102 ... 255] = &&l_err,
    };

    static void* go_arr[] = {
        [0 ... 8]     = &&l_err,
        ['\t']        = &&l_next,
        ['\n']        = &&l_next,
        [11 ... 12]   = &&l_err,
        ['\r']        = &&l_next,
        [14 ... 31]   = &&l_err,
        [' ']         = &&l_next,
        [33 ... 33]   = &&l_err,
        ['"']         = &&l_str_s,
        [35 ... 43]   = &&l_err,
        [',']         = &&l_next,
        ['-']         = &&l_num_s,
        [46 ... 47]   = &&l_err,
        ['0' ... '9'] = &&l_num_s,
        [58 ... 90]   = &&l_err,
        ['[']         = &&l_arr_s,
        [92 ... 92]   = &&l_err,
        [']']         = &&l_arr_e,
        [94 ... 101]  = &&l_err,
        ['f']         = &&l_false_f,
        [103 ... 109] = &&l_err,
        ['n']         = &&l_null_n,
        [111 ... 115] = &&l_err,
        ['t']         = &&l_true_t,
        [117 ... 122] = &&l_err,
        ['{']         = &&l_obj_s,
        [124 ... 255] = &&l_err,
    };

    static void* go_obj[] = {
        [0 ... 8]     = &&l_err,
        ['\t']        = &&l_next,
        ['\n']        = &&l_next,
        [11 ... 12]   = &&l_err,
        ['\r']        = &&l_next,
        [14 ... 31]   = &&l_err,
        [' ']         = &&l_next,
        [33 ... 33]   = &&l_err,
        ['"']         = &&l_kvp,
        [35 ... 43]   = &&l_err,
        [',']         = &&l_next,
        [45 ... 124]  = &&l_err,
        ['}']         = &&l_obj_e,
        [126 ... 255] = &&l_err,
    };

    static void* go_col[] = {
        [0 ... 8]     = &&l_err,
        ['\t']        = &&l_next,
        ['\n']        = &&l_next,
        [11 ... 12]   = &&l_err,
        ['\r']        = &&l_next,
        [14 ... 31]   = &&l_err,
        [' ']         = &&l_next,
        [33 ... 57]   = &&l_err,
        [':']         = &&l_col,
        [59 ... 255]  = &&l_err,
    };

    static void* go_obj_val[] = {
        [0 ... 8]     = &&l_err,
        ['\t']        = &&l_next,
        ['\n']        = &&l_next,
        [11 ... 12]   = &&l_err,
        ['\r']        = &&l_next,
        [14 ... 31]   = &&l_err,
        [' ']         = &&l_next,
        [33 ... 255]  = &&l_val,
    };

    jr_str data = { .cstr = 0, .len = 0 };

    const char* c = NULL;

    void**  go = go_doc;
    void**  go_stack[255];
    int32_t go_stack_idx = 0;
    int32_t utf8_mask = 0;

l_next:
    JR_DISPATCH_NEXT();

l_err:
    data.cstr = c;
    data.len = 1;
    cb(jr_type_error, &data, user_data);
    return;

l_num_s:
    data.cstr = c;
    JR_PUSH_GO(go_num);
    JR_DISPATCH_NEXT();

l_num_e:
    data.len = (int32_t)(c - data.cstr);
    cb(jr_type_number, &data, user_data);
    JR_POP_GO();
    JR_DISPATCH_THIS();

l_str_s:
    data.cstr = cstr;
    JR_PUSH_GO(go_str);
    JR_DISPATCH_NEXT();

l_str_e:
    data.len = (int32_t)(c - data.cstr);
    cb(jr_type_string, &data, user_data);
    JR_POP_GO();
    JR_DISPATCH_NEXT();

l_utf8:
    utf8_mask >>= 8;
    goto *go_utf8[*(c = cstr++) & utf8_mask];

l_utf8_2:
    utf8_mask = 0x000000FF;
    goto *go_utf8[*(c = cstr++)];

l_utf8_3:
    utf8_mask = 0x0000FFFF;
    goto *go_utf8[*(c = cstr++)];

l_utf8_4:
    utf8_mask = 0x00FFFFFF;
    goto *go_utf8[*(c = cstr++)];

l_utf8_valid:
    goto *go_utf8_valid[utf8_mask];

l_null_n:
    JR_PUSH();
    goto *go_null_n[*(c = cstr++)];
    
l_null_u:
    goto *go_null_u[*(c = cstr++)];
    
l_null_l:
    goto *go_null_l[*(c = cstr++)];

l_null_ll:
    cb(jr_type_null, NULL, user_data);
    JR_POP_GO();
    JR_DISPATCH_NEXT();

l_true_t:
    JR_PUSH();
    goto *go_true_t[*(c = cstr++)];
    
l_true_r:
    goto *go_true_r[*(c = cstr++)];
    
l_true_u:
    goto *go_true_u[*(c = cstr++)];

l_true_e:
    cb(jr_type_true, NULL, user_data);
    JR_POP_GO();
    JR_DISPATCH_NEXT();

l_false_f:
    JR_PUSH();
    goto *go_false_f[*(c = cstr++)];

l_false_a:
    goto *go_false_a[*(c = cstr++)];
    
l_false_l:
    goto *go_false_l[*(c = cstr++)];
    
l_false_s:
    goto *go_false_s[*(c = cstr++)];
    
l_false_e:
    cb(jr_type_false, NULL, user_data);
    JR_POP_GO();
    JR_DISPATCH_NEXT();

l_arr_s:
    cb(jr_type_array_start, NULL, user_data);
    JR_PUSH_GO(go_arr);
    JR_DISPATCH_NEXT();

l_arr_e:
    cb(jr_type_array_end, NULL, user_data);
    JR_POP_GO();
    JR_DISPATCH_NEXT();

l_obj_s:
    cb(jr_type_object_start, NULL, user_data);
    JR_PUSH_GO(go_obj);
    JR_DISPATCH_NEXT();

l_obj_e:
    cb(jr_type_object_end, NULL, user_data);
    JR_POP_GO();
    JR_DISPATCH_NEXT();

l_kvp:
    data.cstr = cstr;
    JR_PUSH_GO(go_obj_val);
    JR_PUSH_GO(go_col);
    JR_PUSH_GO(go_key);
    JR_DISPATCH_NEXT();

l_key:
    data.len = (int32_t)(c - data.cstr);
    cb(jr_type_key, &data, user_data);
    JR_POP_GO();
    JR_DISPATCH_NEXT();

l_val:
    JR_POP_GO();
    goto *go_val[*c];

l_col:
    JR_POP_GO();
    JR_DISPATCH_NEXT();

l_done:
    return;
}

