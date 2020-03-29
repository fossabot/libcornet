/*
 * Copyright 2020 Alex Syrnikov <pioneer19@post.cz>
 *
 * This file is part of libcornet.
 *
 *  libcornet is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libcornet is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libcornet.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <algorithm>

#include <openssl/evp.h>

#include <doctest/doctest.h>

#include <iostream> // FIXME: remove this line
#include <libcornet/tls/types.hpp>
namespace record = pioneer19::cornet::tls13::record;
#include <libcornet/tls/crypto/record_ciphers.hpp>
namespace tls_crypto = pioneer19::cornet::tls13::crypto;

TEST_CASE("openssl aes_128_gcm key and iv size is good")
{
    tls_crypto::TlsCipherSuite aes_128_gcm;
    aes_128_gcm.set_cipher_suite( record::TLS_AES_128_GCM_SHA256 );

    CHECK( EVP_CIPHER_key_length( EVP_aes_128_gcm() ) == aes_128_gcm.key_size() );
    CHECK( EVP_CIPHER_iv_length(  EVP_aes_128_gcm() ) == aes_128_gcm.iv_size()  );
}

TEST_CASE("Tls13_aes_128_gcm_sha256 encrypt then decrypt in same buffer")
{
    tls_crypto::TlsCipherSuite aes_128_gcm;
    aes_128_gcm.set_cipher_suite( record::TLS_AES_128_GCM_SHA256 );
    const uint8_t plaintext[] = "Hello, world! // tls aes_128_gcm!!!"
                                "Hello, world! // tls aes_128_gcm!!!again";
    const uint8_t aad[] = "today is good day";
    const uint8_t key[16] = {
            0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f };
    const uint8_t iv[12] = {
            0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b };
    std::copy_n( key, sizeof(key), aes_128_gcm.sender_key_data() );
    std::copy_n( key, sizeof(key), aes_128_gcm.receiver_key_data() );
    std::copy_n( iv, sizeof(iv),   aes_128_gcm.sender_iv_data() );
    std::copy_n( iv, sizeof(iv),   aes_128_gcm.receiver_iv_data() );
    uint8_t tag[16];
    uint8_t ciphertext[1024];
    uint32_t ciphertext_size = aes_128_gcm.encrypt( plaintext, sizeof(plaintext)-1
                                                     ,aad, sizeof(aad)-1//, key, iv
                                                     ,ciphertext, tag );
    std::cout << "plaintext size " << sizeof(plaintext)-1
              << ", encrypted ciphertext size " << ciphertext_size << "\n";

//    uint8_t decrypted[1024];
    uint8_t* decrypted = ciphertext;
    uint32_t decrypted_size = aes_128_gcm.decrypt( ciphertext, ciphertext_size
                                                    ,aad, sizeof(aad)-1, tag, decrypted );
    std::cout << "decrypted plaintext size " << decrypted_size << "\n";
    std::string dec_string( (const char*)decrypted, decrypted_size );
    std::cout << "decrypted: " << dec_string << "\n";

    CHECK( decrypted_size == sizeof(plaintext)-1 );
}

TEST_CASE("Tls13_aes_128_gcm_sha256 encrypt then decrypt")
{
    tls_crypto::TlsCipherSuite aes_128_gcm;
    aes_128_gcm.set_cipher_suite( record::TLS_AES_128_GCM_SHA256 );
    const uint8_t plaintext[] = "Hello, world! // tls aes_128_gcm!!!";
    const uint8_t aad[] = "today is good day";
    const uint8_t key[16] = {
            0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f };
    const uint8_t iv[12] = {
            0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b };
    std::copy_n( key, sizeof(key), aes_128_gcm.sender_key_data() );
    std::copy_n( key, sizeof(key), aes_128_gcm.receiver_key_data() );
    std::copy_n( iv, sizeof(iv),   aes_128_gcm.sender_iv_data() );
    std::copy_n( iv, sizeof(iv),   aes_128_gcm.receiver_iv_data() );
    uint8_t tag[16];
    uint8_t ciphertext[1024];

    uint32_t ciphertext_size = 0;
    SUBCASE( "encrypt whole block at once" )
    {
        ciphertext_size = aes_128_gcm.encrypt( plaintext, sizeof( plaintext ) - 1
                                                , aad, sizeof( aad ) - 1
                                                , ciphertext, tag );
        std::cout << "plaintext size " << sizeof( plaintext ) - 1
                  << ", encrypted ciphertext size " << ciphertext_size << "\n";
    }
    SUBCASE( "encrypt2 (two parts)" )
    {
        uint32_t tail_size = sizeof(plaintext)/4;
        const uint8_t* plaintext_tail = plaintext+(sizeof(plaintext)-1-tail_size);
        ciphertext_size = aes_128_gcm.encrypt2( plaintext, sizeof( plaintext ) - 1 - tail_size
                                                 ,plaintext_tail, tail_size
                                                 , aad, sizeof( aad ) - 1
                                                 , ciphertext, tag );
        std::cout << "plaintext size " << sizeof( plaintext ) - 1
                  << ", encrypted ciphertext size " << ciphertext_size << "\n";
    }

    uint8_t decrypted[1024];
    uint32_t decrypted_size = aes_128_gcm.decrypt( ciphertext, ciphertext_size
            ,aad, sizeof(aad)-1, tag,/* key, iv,*/ decrypted );
    std::cout << "decrypted plaintext size " << decrypted_size << "\n";
    std::string dec_string( (const char*)decrypted, decrypted_size );
    std::cout << "decrypted: " << dec_string << "\n";

    CHECK( decrypted_size == sizeof(plaintext)-1 );
}

TEST_CASE("openssl aes_256_gcm key and iv size is good")
{
    tls_crypto::TlsCipherSuite aes_256_gcm;
    aes_256_gcm.set_cipher_suite( record::TLS_AES_256_GCM_SHA384 );

    CHECK( EVP_CIPHER_key_length(EVP_aes_256_gcm() ) == aes_256_gcm.key_size() );
    CHECK( EVP_CIPHER_iv_length( EVP_aes_256_gcm() ) == aes_256_gcm.iv_size() );

    CHECK( EVP_MD_size( EVP_sha384() ) == aes_256_gcm.digest_size() );
}

TEST_CASE("Tls13_aes_256_gcm_sha384 encrypt then decrypt")
{
    tls_crypto::TlsCipherSuite aes_256_gcm;
    aes_256_gcm.set_cipher_suite( record::TLS_AES_256_GCM_SHA384 );
    const uint8_t plaintext[] = "Hello, world! // tls aes_256_gcm!!!";
    const uint8_t aad[] = "today is good day";
    const uint8_t key[32] = {
            0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f
            ,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f };
    const uint8_t iv[12] = {
            0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b };
    std::copy_n( key, sizeof(key), aes_256_gcm.sender_key_data() );
    std::copy_n( key, sizeof(key), aes_256_gcm.receiver_key_data() );
    std::copy_n( iv, sizeof(iv),   aes_256_gcm.sender_iv_data() );
    std::copy_n( iv, sizeof(iv),   aes_256_gcm.receiver_iv_data() );
    uint8_t tag[16];
    uint8_t ciphertext[1024];
    uint32_t ciphertext_size = 0;

    SUBCASE( "encrypt whole block at once" )
    {
        ciphertext_size = aes_256_gcm.encrypt(
                plaintext, sizeof( plaintext ) - 1
                , aad, sizeof( aad ) - 1//, key, iv
                , ciphertext, tag );
        std::cout << "plaintext size " << sizeof( plaintext ) - 1
                  << ", encrypted ciphertext size " << ciphertext_size << "\n";
    }
    SUBCASE( "encrypt2 (two parts)" )
    {
        uint32_t tail_size = sizeof( plaintext ) / 4;
        const uint8_t* plaintext_tail = plaintext + (sizeof( plaintext ) - 1 - tail_size);
        ciphertext_size = aes_256_gcm.encrypt2( plaintext, sizeof( plaintext ) - 1 - tail_size
                                                 , plaintext_tail, tail_size
                                                 , aad, sizeof( aad ) - 1//, key, iv
                                                 , ciphertext, tag );
        std::cout << "plaintext size " << sizeof( plaintext ) - 1
                  << ", encrypted ciphertext size " << ciphertext_size << "\n";
    }

    uint8_t decrypted[1024];
    uint32_t decrypted_size = aes_256_gcm.decrypt(
            ciphertext, ciphertext_size, aad, sizeof(aad)-1, tag,/* key, iv,*/ decrypted );
    std::cout << "decrypted plaintext size " << decrypted_size << "\n";
    std::string dec_string( (const char*)decrypted, decrypted_size );
    std::cout << "decrypted: " << dec_string << "\n";

    CHECK( decrypted_size == sizeof(plaintext)-1 );
}

TEST_CASE("openssl chacha20_poly1305 key and iv size is good")
{
    tls_crypto::TlsCipherSuite chacha20_poly1305;
    chacha20_poly1305.set_cipher_suite( record::TLS_CHACHA20_POLY1305_SHA256 );

    CHECK( EVP_CIPHER_key_length(EVP_chacha20_poly1305()) == chacha20_poly1305.key_size() );
    CHECK( EVP_CIPHER_iv_length(EVP_chacha20_poly1305()) == chacha20_poly1305.iv_size() );

    CHECK( EVP_MD_size( EVP_sha256() ) == chacha20_poly1305.digest_size() );
}

TEST_CASE("Tls13_chacha20_poly1305_sha256 encrypt then decrypt")
{
    tls_crypto::TlsCipherSuite chacha20_poly1305;
    chacha20_poly1305.set_cipher_suite( record::TLS_CHACHA20_POLY1305_SHA256 );

    const uint8_t plaintext[] = "Hello, world! // tls chacha20_poly1305!!!";
    const uint8_t aad[] = "today is good day";
    const uint8_t key[32] = {
            0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f
            ,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f };
    const uint8_t iv[12] = {
            0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b };
    std::copy_n( key, sizeof(key), chacha20_poly1305.sender_key_data() );
    std::copy_n( key, sizeof(key), chacha20_poly1305.receiver_key_data() );
    std::copy_n( iv, sizeof(iv), chacha20_poly1305.sender_iv_data() );
    std::copy_n( iv, sizeof(iv), chacha20_poly1305.receiver_iv_data() );

    uint8_t tag[16];
    uint8_t ciphertext[1024];
    uint32_t ciphertext_size = 0;

    SUBCASE( "encrypt whole block at once" )
    {
        ciphertext_size = chacha20_poly1305.encrypt(
                plaintext, sizeof( plaintext ) - 1
                , aad, sizeof( aad ) - 1//, key, iv
                , ciphertext, tag );
        std::cout << "plaintext size " << sizeof( plaintext ) - 1
                  << ", encrypted ciphertext size " << ciphertext_size << "\n";
    }
    SUBCASE( "encrypt2 (two parts)" )
    {
        uint32_t tail_size = sizeof( plaintext ) / 4;
        const uint8_t* plaintext_tail = plaintext + (sizeof( plaintext ) - 1 - tail_size);
        ciphertext_size = chacha20_poly1305.encrypt2(
                plaintext, sizeof( plaintext ) - 1 - tail_size
                , plaintext_tail, tail_size
                , aad, sizeof( aad ) - 1//, key, iv
                , ciphertext, tag );
        std::cout << "plaintext size " << sizeof( plaintext ) - 1
                  << ", encrypted ciphertext size " << ciphertext_size << "\n";
    }

    uint8_t decrypted[1024];
    uint32_t decrypted_size = chacha20_poly1305.decrypt(
            ciphertext, ciphertext_size, aad, sizeof(aad)-1, tag,/* key, iv,*/ decrypted );
    std::cout << "decrypted plaintext size " << decrypted_size << "\n";
    std::string dec_string( (const char*)decrypted, decrypted_size );
    std::cout << "decrypted: " << dec_string << "\n";

    CHECK( decrypted_size == sizeof(plaintext)-1 );
}

//TEST_CASE("hkdf_extract check rfc5869 test case 1")
//{
//    /*
//     * rfc5869 test case 1
//     * Basic test case with SHA-256
//     * Hash = SHA-256
//     * IKM  = 0x0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b (22 octets)
//     * salt = 0x000102030405060708090a0b0c (13 octets)
//     * info = 0xf0f1f2f3f4f5f6f7f8f9 (10 octets)
//     * L    = 42
//     *
//     * PRK  = 0x077709362c2e32df0ddc3f0dc47bba63
//     *        90b6c73bb50f9c3122ec844ad7c2b3e5 (32 octets)
//     * OKM  = 0x3cb25f25faacd57a90434f64d0362f2a
//     *        2d2d0a90cf1a5a4c5db02d56ecc4c5bf
//     *        34007208d5b887185865 (42 octets)
//     */
//    const EVP_MD* md = EVP_sha256();
//
//    unsigned char ikm[] = { 0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b
//                            ,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b };
//    unsigned char salt[] = { 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c };
//    unsigned char info[] = { 0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9 };
//    constexpr unsigned l = 42;
//    unsigned char prk_expected[] =
//            { 0x07,0x77,0x09,0x36,0x2c,0x2e,0x32,0xdf,0x0d,0xdc,0x3f,0x0d,0xc4,0x7b, 0xba,0x63
//              ,0x90,0xb6,0xc7,0x3b,0xb5,0x0f,0x9c,0x31,0x22,0xec,0x84,0x4a,0xd7,0xc2,0xb3,0xe5 };
//    unsigned char okm_expected[] =
//            { 0x3c,0xb2,0x5f,0x25,0xfa,0xac,0xd5,0x7a,0x90,0x43,0x4f,0x64,0xd0,0x36,0x2f,0x2a
//              ,0x2d,0x2d,0x0a,0x90,0xcf,0x1a,0x5a,0x4c,0x5d,0xb0,0x2d,0x56,0xec,0xc4,0xc5,0xbf
//              ,0x34,0x00,0x72,0x08,0xd5,0xb8,0x87,0x18,0x58,0x65 };
//    unsigned char prk[SHA256_DIGEST_LENGTH];
//    unsigned char okm[l] = {};
//
//    auto prk_size = tls_crypto::hkdf_extract( md, salt, sizeof(salt), ikm, sizeof(ikm), prk );
//
//    REQUIRE( prk_size == SHA256_DIGEST_LENGTH );
//    auto mismatch_pair = std::mismatch( prk_expected, prk_expected + sizeof( prk_expected )
//                                        ,prk );
//    REQUIRE( mismatch_pair.first == prk_expected + sizeof( prk_expected ) );
//    REQUIRE( mismatch_pair.second == prk + sizeof( prk_expected ) );
//
//    auto okm_size = tls_crypto::hkdf_expand( md, prk, prk_size, info, sizeof(info), okm, l );
//    REQUIRE( okm_size == l );
//
//    mismatch_pair = std::mismatch( okm_expected, okm_expected + sizeof(okm_expected)
//                                   ,okm );
//    REQUIRE( mismatch_pair.first == okm_expected + sizeof(okm_expected) );
//    REQUIRE( mismatch_pair.second == okm + sizeof(okm_expected) );
//}
//
//TEST_CASE("hkdf_extract check rfc5869 test case 2")
//{
//    /*
//     * Test with SHA-256 and longer inputs/outputs
//     *
//     * Hash = SHA-256
//     *
//     * IKM  = 0x000102030405060708090a0b0c0d0e0f
//     * 101112131415161718191a1b1c1d1e1f
//     * 202122232425262728292a2b2c2d2e2f
//     * 303132333435363738393a3b3c3d3e3f
//     * 404142434445464748494a4b4c4d4e4f (80 octets)
//     *
//     * salt = 0x606162636465666768696a6b6c6d6e6f
//     * 707172737475767778797a7b7c7d7e7f
//     * 808182838485868788898a8b8c8d8e8f
//     * 909192939495969798999a9b9c9d9e9f
//     * a0a1a2a3a4a5a6a7a8a9aaabacadaeaf (80 octets)
//     *
//     * info = 0xb0b1b2b3b4b5b6b7b8b9babbbcbdbebf
//     * c0c1c2c3c4c5c6c7c8c9cacbcccdcecf
//     * d0d1d2d3d4d5d6d7d8d9dadbdcdddedf
//     * e0e1e2e3e4e5e6e7e8e9eaebecedeeef
//     * f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff (80 octets)
//     * L = 82
//     *
//     * PRK  = 0x06a6b88c5853361a06104c9ceb35b45c
//     * ef760014904671014a193f40c15fc244 (32 octets)
//     *
//     * OKM  = 0xb11e398dc80327a1c8e7f78c596a4934
//     * 4f012eda2d4efad8a050cc4c19afa97c
//     * 59045a99cac7827271cb41c65e590e09
//     * da3275600c2f09b8367793a9aca3db71
//     * cc30c58179ec3e87c14c01d5c1f3434f
//     * 1d87 (82 octets)
//
//     */
//    const EVP_MD* md = EVP_sha256();
//
//    unsigned char ikm[] = { 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f
//                            ,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f
//                            ,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f
//                            ,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f
//                            ,0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f };
//    unsigned char salt[] = { 0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f
//                             ,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f
//                             ,0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f
//                             ,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f
//                             ,0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf };
//    unsigned char info[] = { 0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf
//                             ,0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf
//                             ,0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf
//                             ,0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef
//                             ,0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff };
//    constexpr unsigned l = 82;
//    unsigned char prk_expected[] =
//            { 0x06,0xa6,0xb8,0x8c,0x58,0x53,0x36,0x1a,0x06,0x10,0x4c,0x9c,0xeb,0x35,0xb4,0x5c
//              ,0xef,0x76,0x00,0x14,0x90,0x46,0x71,0x01,0x4a,0x19,0x3f,0x40,0xc1,0x5f,0xc2,0x44 };
//    unsigned char okm_expected[] =
//            { 0xb1,0x1e,0x39,0x8d,0xc8,0x03,0x27,0xa1,0xc8,0xe7,0xf7,0x8c,0x59,0x6a,0x49,0x34
//              ,0x4f,0x01,0x2e,0xda,0x2d,0x4e,0xfa,0xd8,0xa0,0x50,0xcc,0x4c,0x19,0xaf,0xa9,0x7c
//              ,0x59,0x04,0x5a,0x99,0xca,0xc7,0x82,0x72,0x71,0xcb,0x41,0xc6,0x5e,0x59,0x0e,0x09
//              ,0xda,0x32,0x75,0x60,0x0c,0x2f,0x09,0xb8,0x36,0x77,0x93,0xa9,0xac,0xa3,0xdb,0x71
//              ,0xcc,0x30,0xc5,0x81,0x79,0xec,0x3e,0x87,0xc1,0x4c,0x01,0xd5,0xc1,0xf3,0x43,0x4f
//              ,0x1d,0x87 };
//    unsigned char prk[SHA256_DIGEST_LENGTH];
//    unsigned char okm[l] = {};
//
//    auto prk_size = tls_crypto::hkdf_extract( md, salt, sizeof(salt), ikm, sizeof(ikm), prk );
//
//    REQUIRE( prk_size == SHA256_DIGEST_LENGTH );
//    auto mismatch_pair = std::mismatch( prk_expected, prk_expected + sizeof( prk_expected )
//                                        ,prk );
//    REQUIRE( mismatch_pair.first == prk_expected + sizeof( prk_expected ) );
//    REQUIRE( mismatch_pair.second == prk + sizeof( prk_expected ) );
//
//    auto okm_size = tls_crypto::hkdf_expand( md, prk, prk_size, info, sizeof(info), okm, l );
//    REQUIRE( okm_size == l );
//
//    mismatch_pair = std::mismatch( okm_expected, okm_expected + sizeof(okm_expected)
//                                   ,okm );
//    REQUIRE( mismatch_pair.first == okm_expected + sizeof(okm_expected) );
//    REQUIRE( mismatch_pair.second == okm + sizeof(okm_expected) );
//}
//
//TEST_CASE("hkdf_extract check rfc5869 test case 3")
//{
//    /*
//     * rfc5869 test case 3
//     * Test with SHA-256 and zero-length salt/info
//     *
//     * Hash = SHA-256
//     * IKM  = 0x0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b (22 octets)
//     * salt = (0 octets)
//     * info = (0 octets)
//     * L    = 42
//     *
//     * PRK  = 0x19ef24a32c717b167f33a91d6f648bdf
//     * 96596776afdb6377ac434c1c293ccb04 (32 octets)
//     * OKM  = 0x8da4e775a563c18f715f802a063c5a31
//     * b8a11f5c5ee1879ec3454e5f3c738d2d
//     * 9d201395faa4b61a96c8 (42 octets)
//     */
//    const EVP_MD* md = EVP_sha256();
//
//    unsigned char ikm[] = { 0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b
//                            ,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b };
////    unsigned char salt[] = {};
//    unsigned char* salt = nullptr;
////    unsigned char info[] = {};
//    unsigned char* info = nullptr;
//    constexpr unsigned l = 42;
//    unsigned char prk_expected[] =
//            { 0x19,0xef,0x24,0xa3,0x2c,0x71,0x7b,0x16,0x7f,0x33,0xa9,0x1d,0x6f,0x64,0x8b,0xdf
//              ,0x96,0x59,0x67,0x76,0xaf,0xdb,0x63,0x77,0xac,0x43,0x4c,0x1c,0x29,0x3c,0xcb,0x04 };
//    unsigned char okm_expected[] =
//            { 0x8d,0xa4,0xe7,0x75,0xa5,0x63,0xc1,0x8f,0x71,0x5f,0x80,0x2a,0x06,0x3c,0x5a,0x31
//              ,0xb8,0xa1,0x1f,0x5c,0x5e,0xe1,0x87,0x9e,0xc3,0x45,0x4e,0x5f,0x3c,0x73,0x8d,0x2d
//              ,0x9d,0x20,0x13,0x95,0xfa,0xa4,0xb6,0x1a,0x96,0xc8 };
//    unsigned char prk[SHA256_DIGEST_LENGTH];
//    unsigned char okm[l] = {};
//
//    auto prk_size = tls_crypto::hkdf_extract( md, salt, 0, ikm, sizeof(ikm), prk );
//
//    REQUIRE( prk_size == SHA256_DIGEST_LENGTH );
//    auto mismatch_pair = std::mismatch( prk_expected, prk_expected + sizeof( prk_expected )
//                                        ,prk );
//    REQUIRE( mismatch_pair.first == prk_expected + sizeof( prk_expected ) );
//    REQUIRE( mismatch_pair.second == prk + sizeof( prk_expected ) );
//
//    auto okm_size = tls_crypto::hkdf_expand( md, prk, prk_size, info, 0, okm, l );
//    REQUIRE( okm_size == l );
//
//    mismatch_pair = std::mismatch( okm_expected, okm_expected + sizeof(okm_expected)
//                                   ,okm );
//    REQUIRE( mismatch_pair.first == okm_expected + sizeof(okm_expected) );
//    REQUIRE( mismatch_pair.second == okm + sizeof(okm_expected) );
//}
