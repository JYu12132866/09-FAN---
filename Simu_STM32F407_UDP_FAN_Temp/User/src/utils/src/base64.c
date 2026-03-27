#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "base64.h"

// Base64 字符集
static const char base64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

// Base64 编码函数
char* base64_encode(const unsigned char* data, size_t input_length, size_t* output_length) {
    *output_length = 4 * ((input_length + 2) / 3);

    char* encoded_data = malloc(*output_length + 1); // +1 for null terminator
    if (encoded_data == NULL) return NULL;

    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = base64_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = base64_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = base64_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = base64_table[(triple >> 0 * 6) & 0x3F];
    }

    // 处理填充
    for (size_t i = 0; i < (3 - input_length % 3) % 3; i++) {
        encoded_data[*output_length - 1 - i] = '=';
    }

    encoded_data[*output_length] = '\0'; // Null-terminated
    return encoded_data;
}

// Base64 解码函数
unsigned char* base64_decode(const char* data, size_t input_length, size_t* output_length) {
    if (input_length % 4 != 0) return NULL;

    *output_length = input_length / 4 * 3;
    if (input_length > 0 && data[input_length - 1] == '=') (*output_length)--;
    if (input_length > 1 && data[input_length - 2] == '=') (*output_length)--;

    unsigned char* decoded_data = malloc(*output_length);
    if (decoded_data == NULL) return NULL;

    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t sextet_a = data[i] == '=' ? 0 : strchr(base64_table, data[i]) - base64_table;
        i++;
        uint32_t sextet_b = data[i] == '=' ? 0 : strchr(base64_table, data[i]) - base64_table;
        i++;
        uint32_t sextet_c = data[i] == '=' ? 0 : strchr(base64_table, data[i]) - base64_table;
        i++;
        uint32_t sextet_d = data[i] == '=' ? 0 : strchr(base64_table, data[i]) - base64_table;
        i++;

        uint32_t triple = (sextet_a << 3 * 6) + (sextet_b << 2 * 6) + (sextet_c << 1 * 6) + (sextet_d << 0 * 6);

        if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return decoded_data;
}

#define TEST_BASE64 // 定义此宏以运行测试代码
// 示例使用
#ifdef TEST_BASE64
//base64测试函数，原始输入是0-255的整数数组，需要编码后打印，再解码后打印出来，看是否一致。

int test_base64(){
    unsigned char data[256];
    for (int i = 0; i < 256; ++i) {
        data[i] = (unsigned char)i;
    }

    size_t output_length;
    char* encoded_data = base64_encode(data, sizeof(data), &output_length);
    if (!encoded_data) return -1;

    printf("Encoded: %s\n", encoded_data);

    unsigned char* decoded_data = base64_decode(encoded_data, output_length, &output_length);
    if (!decoded_data) return -1;
    printf("Decoded: ");
    for (size_t i = 0; i < output_length; ++i) {
        if(decoded_data[i] != data[i]) return -1;
        printf("%x", decoded_data[i]);
    }
    printf("\n");
    free(encoded_data);
    free(decoded_data);
    return 0;
}
#endif