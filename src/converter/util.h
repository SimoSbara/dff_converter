#pragma once
#define _USE_MATH_DEFINES
#include <math.h>

#define DEGREES_RADIANS M_PI / 180.0f

template<class T>
void checkMinMax(T& min, T& max, T v)
{
    if (min > v)
        min = v;
    if (max < v)
        max = v;
}

template<class T>
void checkMinMaxInd(T& min, T& max, int& minInd, int& maxInd, T v, int i)
{
    if (min > v)
    {
        min = v;
        minInd = i;
    }
    if (max < v)
    {
        max = v;
        maxInd = i;
    }
}

static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

static std::string base64_encode(unsigned char const* bytes_to_encode,
    unsigned int in_len) {
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    const char* base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] =
                ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] =
                ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++) ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++) char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] =
            ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] =
            ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; (j < i + 1); j++) ret += base64_chars[char_array_4[j]];

        while ((i++ < 3)) ret += '=';
    }

    return ret;
}

static std::string base64_decode(std::string const& encoded_string) {
    int in_len = static_cast<int>(encoded_string.size());
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    while (in_len-- && (encoded_string[in_] != '=') &&
        is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] =
                static_cast<unsigned char>(base64_chars.find(char_array_4[i]));

            char_array_3[0] =
                (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] =
                ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++) ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++) char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] =
            static_cast<unsigned char>(base64_chars.find(char_array_4[j]));

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] =
            ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}

//wikipedia da eulero in gradi a quaternione
static void convertEulerToQuaternion(float rx, float ry, float rz, float& qx, float& qy, float& qz, float& qw)
{
    //converto in radianti
    rx *= DEGREES_RADIANS;
    ry *= DEGREES_RADIANS;
    rz *= DEGREES_RADIANS;

    double cr = cos(rx * 0.5);
    double sr = sin(rx * 0.5);
    double cp = cos(ry * 0.5);
    double sp = sin(ry * 0.5);
    double cy = cos(rz * 0.5);
    double sy = sin(rz * 0.5);

    qw = cr * cp * cy + sr * sp * sy;
    qx = sr * cp * cy - cr * sp * sy;
    qy = cr * sp * cy + sr * cp * sy;
    qz = cr * cp * sy - sr * sp * cy;
}

//https://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
static void convertMat3x3ToQuaternion(float* mat, float& qx, float& qy, float& qz, float& qw)
{
    if (mat == NULL)
        return;

    float m00 = mat[0];
    float m01 = mat[1];
    float m02 = mat[2];

    float m10 = mat[3];
    float m11 = mat[4];
    float m12 = mat[5];

    float m20 = mat[6];
    float m21 = mat[7];
    float m22 = mat[8];

    float tr = m00 + m11 + m22; //diagonale

    if (tr > 0) {
        float S = sqrt(tr + 1.0) * 2; // S=4*qw 
        qw = 0.25 * S;
        qx = (m21 - m12) / S;
        qy = (m02 - m20) / S;
        qz = (m10 - m01) / S;
    }
    else if ((m00 > m11) && (m00 > m22)) {
        float S = sqrt(1.0 + m00 - m11 - m22) * 2; // S=4*qx 
        qw = (m21 - m12) / S;
        qx = 0.25 * S;
        qy = (m01 + m10) / S;
        qz = (m02 + m20) / S;
    }
    else if (m11 > m22) {
        float S = sqrt(1.0 + m11 - m00 - m22) * 2; // S=4*qy
        qw = (m02 - m20) / S;
        qx = (m01 + m10) / S;
        qy = 0.25 * S;
        qz = (m12 + m21) / S;
    }
    else {
        float S = sqrt(1.0 + m22 - m00 - m11) * 2; // S=4*qz
        qw = (m10 - m01) / S;
        qx = (m02 + m20) / S;
        qy = (m12 + m21) / S;
        qz = 0.25 * S;
    }
}