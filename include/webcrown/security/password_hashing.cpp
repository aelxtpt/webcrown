#include "webcrown/security/password_hashing.hpp"

#include <fcntl.h>
#include <unistd.h>

#if defined(__APPLE__)
#include <mach/mach.h>
#else
#include <sys/sysinfo.h>
#endif
#include <argon2.h>

void crypto_fill(void* buffer, size_t size)
{
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0)
        throw std::runtime_error("Cannot open /dev/random file for reading!");

    ssize_t count = read(fd, buffer, size);
    if (count < 0)
        throw std::runtime_error("Cannot read from /dev/random file!");

    int result = close(fd);
    if (result != 0)
        throw std::runtime_error("Cannot close dev/random file");

}

std::string base64_encode(std::string_view str)
{
    const char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const size_t mods[] = { 0, 2, 1 };

    size_t ilength = str.length();
    size_t olength = 4 * ((ilength + 2) / 3);

    std::string result;
    result.resize(olength, 0);

    for (size_t i = 0, j = 0; i < ilength;)
    {
        uint32_t octet_a = i < ilength ? (uint8_t)str[i++] : 0;
        uint32_t octet_b = i < ilength ? (uint8_t)str[i++] : 0;
        uint32_t octet_c = i < ilength ? (uint8_t)str[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        result[j++] = base64[(triple >> 3 * 6) & 0x3F];
        result[j++] = base64[(triple >> 2 * 6) & 0x3F];
        result[j++] = base64[(triple >> 1 * 6) & 0x3F];
        result[j++] = base64[(triple >> 0 * 6) & 0x3F];
    }

    for (size_t i = 0; i < mods[ilength % 3]; ++i)
        result[result.size() - 1 - i] = '=';

    return result;
}

std::string base64_decode(std::string_view str)
{
    static const unsigned char base64[256] =
    {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x3f,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
    0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    size_t ilength = str.length();

    if (ilength % 4 != 0)
        return "";

    size_t olength = ilength / 4 * 3;

    if (str[ilength - 1] == '=') olength--;
    if (str[ilength - 2] == '=') olength--;

    std::string result;
    result.resize(olength, 0);

    for (size_t i = 0, j = 0; i < ilength;)
    {
        uint32_t sextet_a = str[i] == '=' ? 0 & i++ : base64[(uint8_t)str[i++]];
        uint32_t sextet_b = str[i] == '=' ? 0 & i++ : base64[(uint8_t)str[i++]];
        uint32_t sextet_c = str[i] == '=' ? 0 & i++ : base64[(uint8_t)str[i++]];
        uint32_t sextet_d = str[i] == '=' ? 0 & i++ : base64[(uint8_t)str[i++]];

        uint32_t triple = (sextet_a << 3 * 6) + (sextet_b << 2 * 6) + (sextet_c << 1 * 6) + (sextet_d << 0 * 6);

        if (j < olength) result[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < olength) result[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < olength) result[j++] = (triple >> 0 * 8) & 0xFF;

    }

    return result;
}

std::string password_hashing::generate_salt() const
{
    std::string salt(salt_length(), 0);
    crypto_fill(salt.data(), salt.size());
    return salt;
}

std::pair<std::string, std::string> password_hashing::generate_hash_and_salt(std::string_view password) const
{
    auto salt = generate_salt();
    auto hash = generate_hash(password, salt);
    return std::make_pair(hash, salt);
}

std::string password_hashing::generate_digest(std::string_view password) const
{
    auto digest = generate_hash_and_salt(password);
    return digest.first + digest.second;
}

std::string password_hashing::generate_encoded_digest(std::string_view password) const
{
    // Encode the digest into the base64 encoding
    return base64_encode(generate_digest(password));
}

bool password_hashing::validate_digest(std::string_view password, std::string digest) const
{
    // Check the digest size ( must be hash + salt)
    if (digest.size() != (hash_length() + salt_length()))
        return false;

    // Extract hash and salt from the digest
    std::string_view hash(digest.data(), hash_length());
    std::string_view salt(digest.data() + hash_length(), salt_length());

    // Perform the password validation
    return validate(password, hash, salt);
}

bool password_hashing::validate_encoded_digest(std::string_view password, std::string_view digest) const
{
    // Decode the digest from the base64 encoding
    return validate_digest(password, base64_decode(digest));
}

argon2d_password_hashing::argon2d_password_hashing(size_t hash_length, size_t salt_length, uint32_t t, uint32_t m,
                                                   uint32_t p)
                                                   : password_hashing(hash_length, salt_length)
                                                   , iterations_num_(t)
                                                   , memory_usage_num_(m)
                                                   , degree_parallelism_(p)
{

}

std::string argon2d_password_hashing::generate_hash(std::string_view password, std::string_view salt) const
{
    // generate the strong password hash
    std::string hash(hash_length(), 0);
    if (argon2d_hash_raw(iterations_num(), memory_usage_num(), degree_parallelism(), password.data(),
                         password.size(), salt.data(), salt.size(), hash.data(), hash.size()) != ARGON2_OK)
        throw std::runtime_error("Cannot generate 'Argon2d' hash!");
    return hash;
}

bool argon2d_password_hashing::validate(std::string_view password, std::string_view hash, std::string_view salt) const
{
    // Calculate the digest for the given password and salt
    std::string digest(hash.size(), 0);
    if (argon2d_hash_raw(iterations_num(), memory_usage_num(), degree_parallelism(), password.data(), password.size(),
                         salt.data(), salt.size(), digest.data(), digest.size()) != ARGON2_OK)
        throw std::runtime_error("Cannot calculate 'Argon2d' hash!");

    // Compare the digest with the given hash
    return (digest == hash);
}