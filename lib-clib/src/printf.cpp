/**
 * @file printf.cpp
 *
 */
/* Copyright (C) 2016-2025 by Arjan van Vught mailto:info@gd32-dmx.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <cstdint>
#include <cstdarg>
#include <cstddef>
#include <cctype>
#include <cstring>
#include <climits>

namespace console
{
void Putc(int);
}

struct Context
{
    int flag;
    int prec;
    int width;
    int total;
    int capacity;
};

enum
{
    kFlagPrecision = (1U << 0),
    kFlagUppercase = (1U << 1),
    kFlagLong = (1U << 2),
    kFlagNegative = (1U << 3),
    kFlagMinWidth = (1U << 4),
    kFlagZeroPadded = (1U << 5),
    kFlagLeftJustified = (1U << 6)
};

static char* outptr = nullptr;

inline static void Putc(struct Context* ctx, int c)
{
    ctx->total++;

    if (outptr != nullptr)
    {
        if (ctx->total < ctx->capacity)
        {
            *outptr++ = static_cast<char>(c);
        }
        return;
    }

    console::Putc(c);
}

static void FormatHex(struct Context* ctx, unsigned int arg)
{
    char buffer[64];
    char* p = buffer + (sizeof(buffer) / sizeof(buffer[0])) - 1;
    char* o = p;
    char alpha;
    char u;
    int i;

    if (arg == 0)
    {
        *p = '0';
        p--;
    }
    else
    {
        alpha = ((ctx->flag & kFlagUppercase) != 0) ? static_cast<char>('A' - 10) : static_cast<char>('a' - 10);

        do
        {
            u = static_cast<char>(arg) & static_cast<char>(0x0F);
            *p = (u < 10) ? static_cast<char>('0' + u) : static_cast<char>(alpha + u);
            p--;
            arg = arg >> 4;
        } while ((arg != 0) && (p > buffer));
    }

    if ((ctx->flag & kFlagPrecision) != 0)
    {
        while (((o - p) < ctx->prec) && (p > buffer))
        {
            *p-- = '0';
        }
    }

    if ((ctx->flag & kFlagZeroPadded) != 0)
    {
        while (((o - p) < ctx->width) && (p > buffer))
        {
            *p-- = '0';
        }
    }

    if ((ctx->flag & kFlagLeftJustified) == 0)
    {
        while (((o - p) < ctx->width) && (p > buffer))
        {
            *p-- = ' ';
        }
    }

    i = o - p;

    p++;

    while (p < buffer + (sizeof(buffer) / sizeof(buffer[0])))
    {
        Putc(ctx, static_cast<int>(*p++));
    }

    while (i++ < ctx->width)
    {
        Putc(ctx, static_cast<int>(' '));
    }
}

static void FormatInt(struct Context* ctx, uint32_t arg)
{
    char buffer[64];
    char* p = buffer + (sizeof(buffer) / sizeof(buffer[0])) - 1;
    char* o = p;
    int i;

    if (arg == 0)
    {
        *p = '0';
        p--;
    }
    else
    {
        do
        {
            *p = static_cast<char>((arg % 10) + '0');
            p--;
            arg = arg / 10;
        } while ((arg != 0) && (p > buffer));
    }

    if ((ctx->flag & kFlagPrecision) != 0)
    {
        while (((o - p) < ctx->prec) && (p > buffer))
        {
            *p-- = '0';
        }
    }

    if ((ctx->flag & kFlagZeroPadded) != 0)
    {
        while (((o - p) < ctx->width) && (p > buffer))
        {
            *p-- = '0';
        }
    }

    if ((ctx->flag & kFlagNegative) != 0)
    {
        *p-- = '-';
    }

    if ((ctx->flag & kFlagLeftJustified) == 0)
    {
        while (((o - p) < ctx->width) && (p > buffer))
        {
            *p-- = ' ';
        }
    }

    i = o - p;

    p++;

    while (p < buffer + (sizeof(buffer) / sizeof(buffer[0])))
    {
        Putc(ctx, static_cast<int>(*p++));
    }

    while (i++ < ctx->width)
    {
        Putc(ctx, static_cast<int>(' '));
    }
}

#if !defined(DISABLE_PRINTF_FLOAT)
static int Pow10(int n)
{
    int r = 10;
    n--;

    while (n-- > 0)
    {
        r *= 10;
    }

    return r;
}

static int Itostr(int x, char* s, int d)
{
    char buffer[64];
    auto* p = buffer + (sizeof(buffer) / sizeof(buffer[0])) - 1;
    auto* o = p;
    auto* t = s;

    const auto kIsNeg = x < 0 ? true : false;

    if (kIsNeg)
    {
        x = -x;
    }

    if (x == 0)
    {
        *p = '0';
        p--;
    }
    else
    {
        do
        {
            *p = static_cast<char>((x % 10) + '0');
            p--;
            x = x / 10;
        } while ((x != 0) && (p > buffer));
    }

    if (d != 0)
    {
        while (((o - p) < d) && (p > buffer))
        {
            *p-- = '0';
        }
    }

    if (kIsNeg)
    {
        *p-- = '-';
    }

    p++;

    auto i = (o - p);

    while (p < buffer + (sizeof(buffer) / sizeof(buffer[0])))
    {
        *t++ = *p++;
    }

    return i + 1;
}

static constexpr int kMaxPrecision = 6;
static constexpr float kFloatRounders[kMaxPrecision + 1] = {
    0.5f, 0.05f, 0.005f, 0.0005f, 0.00005f, 0.000005f, 0.0000005f,
};

static void FormatFloat(struct Context* ctx, float f)
{
    char buffer[64];
    auto* dest = buffer;
    int ipart;
    int precision;
    int size;
    int i;

    if (((ctx->flag & kFlagPrecision) != 0) && (ctx->prec <= kMaxPrecision))
    {
        precision = ctx->prec;
    }
    else
    {
        precision = kMaxPrecision;
    }

    if (f < 0)
    {
        *dest++ = '-';
        f = -f;
    }

    if (precision)
    {
        f += kFloatRounders[precision];
    }

    ipart = static_cast<int>(f);

    dest += Itostr(ipart, dest, 0);

    f -= static_cast<float>(ipart);

    *dest++ = '.';
    dest += Itostr(static_cast<int>(f * static_cast<float>(Pow10(precision))), dest, precision);
    size = dest - buffer;

    i = 0;
    while (((size + i) < ctx->width))
    {
        Putc(ctx, static_cast<int>(' '));
        i++;
    }

    dest = buffer;
    while (size-- > 0)
    {
        Putc(ctx, static_cast<int>(*dest++));
    }
}
#endif

static void FormatString(struct Context* ctx, const char* s)
{
    int j;

    for (j = 0; s[j] != 0; j++); // strlen

    if ((ctx->flag & kFlagPrecision) != 0)
    {
        if (ctx->prec < j)
        {
            j = ctx->prec;
        }
    }

    while ((((ctx->flag & kFlagLeftJustified) == 0)) && (j++ < ctx->width))
    {
        Putc(ctx, ' ');
    }

    while ((((ctx->flag & kFlagPrecision) == 0) || (ctx->prec != 0)) && (*s != 0))
    {
        Putc(ctx, *s++);
        ctx->prec--;
    }

    while (j++ < ctx->width)
    {
        Putc(ctx, ' ');
    }
}

static void FormatPointer(struct Context* ctx, unsigned int arg)
{
    ctx->width = 8;
    ctx->flag = kFlagZeroPadded;

    Putc(ctx, static_cast<int>('0'));
    Putc(ctx, static_cast<int>('x'));

    FormatHex(ctx, arg);
}

static int Vprintf(int size, const char* fmt, va_list va)
{
    struct Context ctx;
#if !defined(DISABLE_PRINTF_FLOAT)
    float f;
#endif
    int32_t l;
    uint32_t lu;
    const char* s;

    ctx.total = 0;
    ctx.capacity = size;

    while (*fmt != 0)
    {
        if (*fmt != '%')
        {
            Putc(&ctx, static_cast<int>(*fmt++));
            continue;
        }

        fmt++;

        ctx.flag = 0;
        ctx.prec = 0;
        ctx.width = 0;

        if (*fmt == '0')
        {
            ctx.flag |= kFlagZeroPadded;
            fmt++;
        }
        else if (*fmt == '-')
        {
            ctx.flag |= kFlagLeftJustified;
            fmt++;
        }

        while (isdigit(static_cast<int>(*fmt)) != 0)
        {
            ctx.width = ctx.width * 10 + (*fmt - '0');
            fmt++;
        }

        if (ctx.width != 0)
        {
            ctx.flag |= kFlagMinWidth;
        }

        if (*fmt == '.')
        {
            fmt++;
            if (*fmt == '*')
            {
                fmt++;
                ctx.prec = va_arg(va, int);
                if (ctx.prec < 0)
                {
                    ctx.prec = -ctx.prec;
                }
            }
            else
            {
                while (isdigit(static_cast<int>(*fmt)) != 0)
                {
                    ctx.prec = ctx.prec * 10 + (*fmt - '0');
                    fmt++;
                }
            }
            ctx.flag |= kFlagPrecision;
        }

        if (*fmt == 'l')
        {
            fmt++;
            ctx.flag |= kFlagLong;
        }

        switch (*fmt)
        {
            case 'c':
                Putc(&ctx, va_arg(va, int));
                break;
            case 'd':
                /*@fallthrough@*/
                /* no break */
            case 'i':
                l = ((ctx.flag & kFlagLong) != 0) ? va_arg(va, long int) : static_cast<int32_t>(va_arg(va, int));
                if (l < 0)
                {
                    ctx.flag |= kFlagNegative;
                    l = -l;
                }
                FormatInt(&ctx, static_cast<uint32_t>(l));
                break;
#if !defined(DISABLE_PRINTF_FLOAT)
            case 'f':
                f = static_cast<float>(va_arg(va, double));
                FormatFloat(&ctx, f);
                break;
#endif
            case 'p':
                FormatPointer(&ctx, va_arg(va, unsigned int));
                break;
            case 's':
                s = va_arg(va, const char*);
                FormatString(&ctx, s);
                break;
            case 'u':
                lu = ((ctx.flag & kFlagLong) != 0) ? va_arg(va, unsigned long int) : va_arg(va, unsigned int);
                FormatInt(&ctx, lu);
                break;
            case 'X':
                ctx.flag |= kFlagUppercase;
                /*@fallthrough@*/
                /* no break */
            case 'x':
                FormatHex(&ctx, va_arg(va, unsigned int));
                break;
            default:
                Putc(&ctx, static_cast<int>(*fmt));
                continue;
        }

        fmt++;
    }

    return ctx.total;
}

extern "C"
{
    int printf(const char* fmt, ...) // NOLINT
    {
        va_list arp;
        va_start(arp, fmt);

        auto i = Vprintf(INT_MAX, fmt, arp);

        va_end(arp);

        return i;
    }

    int vprintf(const char* fmt, va_list arp) // NOLINT
    {
        auto i = Vprintf(INT_MAX, fmt, arp);

        return i;
    }

    int sprintf(char* str, const char* fmt, ...) // NOLINT
    {
        va_list arp;

        outptr = str;
        va_start(arp, fmt);

        auto i = Vprintf(INT_MAX, fmt, arp);

        va_end(arp);

        *outptr = 0;
        outptr = nullptr;

        return i;
    }

    int vsprintf(char* str, const char* fmt, va_list ap) // NOLINT
    {
        outptr = str;

        auto i = Vprintf(INT_MAX, fmt, ap);

        *outptr = 0;
        outptr = nullptr;

        return i;
    }

    int vsnprintf(char* str, size_t size, const char* fmt, va_list ap) // NOLINT
    {
        if (size == 0)
        {
            outptr = nullptr;           // don't write anywhere
            return Vprintf(0, fmt, ap); // just count
        }

        outptr = str;

        auto i = Vprintf((int)size, fmt, ap);

        *outptr = 0;
        outptr = nullptr;

        return i;
    }

    int snprintf(char* str, size_t size, const char* fmt, ...) // NOLINT
    {
        va_list ap;
        va_start(ap, fmt);
        int i = vsnprintf(str, size, fmt, ap);
        va_end(ap);
        return i;
    }
}
