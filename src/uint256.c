
#include <stdint.h>
#include <stdio.h>

#include "secp256k1_utils.h"

void printHex_UInt256(const UInt256* ptr){
    printf("0x%016lx %016lx %016lx %016lx\n",ptr->limbs[3], ptr->limbs[2],ptr->limbs[1],ptr->limbs[0]);
}


void zero_UInt256(UInt256* ptr){
    ptr->limbs[0] = 0;
    ptr->limbs[1] = 0;
    ptr->limbs[2] = 0;
    ptr->limbs[3] = 0;
}

int is_zero_value(const UInt256* t){
    return  t->limbs[0] == 0 &&
            t->limbs[1] == 0 && 
            t->limbs[2] == 0 && 
            t->limbs[3] == 0;
}

//Aに戻り値が入ります
uint64_t add_UInt256(UInt256* A,const  UInt256* B){
    uint64_t carry = 0;
    for(int i = 0; i < 4; i++){
        uint64_t a = A->limbs[i];
        uint64_t b = B->limbs[i];
        uint64_t sum = a + b;
        uint64_t carry_from_sum = (sum < a)? 1 : 0;
        uint64_t final_sum = sum + carry;
        uint64_t carry_from_carry = (final_sum < carry)? 1: 0;
        A->limbs[i] = final_sum;
        carry = carry_from_sum | carry_from_carry; 

    }
    return carry;
}

uint64_t sub_UInt256(UInt256* A,const UInt256* B){
    uint64_t borrow = 0;
    for(int i = 0; i < 4; i++){
        uint64_t a = A->limbs[i];
        uint64_t b = B->limbs[i];
        uint64_t sub = a - b;
        uint64_t borrow_from_sub = (a < sub) ? 1 : 0;
        uint64_t final_sub = sub - borrow;
        uint64_t borrow_from_borrow = (sub < final_sub) ? 1 : 0;
        A->limbs[i] = final_sub;
        borrow = borrow_from_sub | borrow_from_borrow;
    }
    return borrow;
}


int cmp_UInt256(const UInt256* A,const  UInt256* B){
    int result = 0;
    for(int i = 0; i < 4; i++){
        uint64_t a = A->limbs[i];
        uint64_t b = B->limbs[i];

        // 1. その桁だけ見た時の勝敗 (-1, 0, 1)
        // (a > b) はC言語では1, (a < b) は1 になる標準仕様を利用
        int current_res = (a > b) - (a < b); 

        // 2. その桁が「違う」かどうか (0 か 1)
        int is_diff = (a != b); 

        // 3. 分岐なし選択 (Branchless Select)
        // もし is_diff が 1 (違う) なら current_res を採用
        // もし is_diff が 0 (同じ) なら、今までの result を保持
        
        // 数学的な書き方:
        // result = (is_diff * current_res) + ((1 - is_diff) * result);
        
        // ビット演算的な書き方 (こちらの方が一般的):
        // マスクを作る: is_diff が 1 なら 0xFF...FF, 0 なら 0x00...00
        // 注: 単純なキャストではなく、符号拡張を利用するか、条件演算子を使う
        // ここでは可読性とコンパイラ最適化を信じて条件演算子を使うが、
        // 厳密なConstant Timeマニアは ( -is_diff ) & ... を好むわ。
        
        // 今回はわかりやすさ優先で「条件演算子」を使うわ。
        // 最近のコンパイラは `x ? y : z` をCMOV命令（条件付き転送）に変換するから
        // 分岐予測ミスは起きないことが多いけれど、完全を目指すなら以下よ。
        
        int mask = -is_diff; // 1なら0xFFFFFFFF..., 0なら0x00000000
        
        // result = (current_res & mask) | (result & ~mask);
        // マスクがかかっている方だけが生き残る
        result = (current_res & mask) | (result & ~mask);
    }

    return result;
}
void not_UInt256(UInt256* A){
    A->limbs[0] = ~A->limbs[0];
    A->limbs[1] = ~A->limbs[1];
    A->limbs[2] = ~A->limbs[2];
    A->limbs[3] = ~A->limbs[3];
}


int get_bit_UInt256(const UInt256* A, int i){
    return (A->limbs[i >> 6] >> (i & (64 - 1))) & 0x1;
}