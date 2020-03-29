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

const uint8_t tls13_client_hello_record[] = {
        0x16, 0x03, 0x01, 0x00, 0xea,   // TlsPlaintext handshake, legacy version, length
        0x01, 0x00, 0x00, 0xe6,         // ClientHello(1), length(24 bit)
        0x03, 0x03,                     // ProtocolVersion
        0xe9, 0x53, 0xc0, 0xde, 0x38, 0x8c, 0x75, 0x82, // Random 32 bytes
        0xbc, 0x49, 0xd5, 0xb2, 0xec, 0x46, 0x7c, 0x99,
        0x21, 0xc5, 0xdb, 0x64, 0x3c, 0x66, 0x07, 0xa4,
        0x18, 0x0e, 0x4d, 0x2a, 0x1a, 0x23, 0x2b, 0x08,
        0x20,                           // legacy session vector length
        0x99, 0x57, 0x6c, 0xce, 0x6e, 0x83, 0xc0, 0x69,
        0xdc, 0xd9, 0x98, 0x43, 0x07, 0xe2, 0xbe, 0xfc,
        0xb4, 0x38, 0x86, 0x33, 0x00, 0xf5, 0x58, 0x5f,
        0x2b, 0x95, 0xce, 0x6f, 0xfe, 0x42, 0xf5, 0x26,
        0x00, 0x08,                     // CipherSuites vector length
        0x13, 0x02, 0x13, 0x03, 0x13, 0x01, 0x00, 0xff,
        0x01, 0x00,                     // legacy_compression_methods<1..2^8-1>
        // === Extensions ===
        0x00, 0x95,                     // Extension extensions<8..2^16-1>;

        0x00, 0x00, 0x00, 0x14,         // ExtensionType(server_name), data_length(2 bytes)
        0x00, 0x12,                     // ServerNameList vector length
        0x00, 0x00, 0x0f,               // ServerName type(1byte) and length
        0x6d, 0x61, 0x73, 0x74, 0x65, 0x72, 0x73, 0x70,
        0x6c, 0x69, 0x6e, 0x65, 0x2e, 0x65, 0x75,

        0x00, 0x0b, 0x00, 0x04,         // ExtensionType(unknown), data_length(2 bytes)
        0x03, 0x00, 0x01, 0x02,

        0x00, 0x0a, 0x00, 0x0c,         //  supported_groups(10)
        0x00, 0x0a, 0x00, 0x1d, 0x00, 0x17, 0x00, 0x1e,
        0x00, 0x19, 0x00, 0x18,

        0x00, 0x23, 0x00, 0x00,         // unknown extension
        0x00, 0x16, 0x00, 0x00,         // unknown extension
        0x00, 0x17, 0x00, 0x00,         // unknown extension
        0x00, 0x0d, 0x00, 0x1e,         // signature_algorithms(13)
        0x00, 0x1c, 0x04, 0x03, 0x05, 0x03, 0x06, 0x03,
        0x08, 0x07, 0x08, 0x08, 0x08, 0x09, 0x08, 0x0a,
        0x08, 0x0b, 0x08, 0x04, 0x08, 0x05, 0x08, 0x06,
        0x04, 0x01, 0x05, 0x01, 0x06, 0x01,

        0x00, 0x2b, 0x00, 0x03,         // supported_versions(43),
        0x02, 0x03, 0x04,               // length, TLS 1.3 (0x03, 0x04)

        0x00, 0x2d, 0x00, 0x02,         // psk_key_exchange_modes(45)
        0x01, 0x01,                     // length 1, mode PSK_DHE_KE = 1

        0x00, 0x33, 0x00, 0x26,         // key_share(51)
        0x00, 0x24, 0x00, 0x1d, 0x00, 0x20, 0xcd, 0xbe,
        0xc4, 0xf3, 0x5a, 0x48, 0x28, 0x6e, 0x59, 0xb0,
        0xe7, 0xeb, 0x2e, 0xe5, 0xa0, 0x51, 0x05, 0x21,
        0x45, 0x7e, 0xdf, 0xa1, 0x12, 0x69, 0x23, 0x42,
        0x2e, 0x92, 0x38, 0xcd, 0xd5, 0x0e
};