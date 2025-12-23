#pragma once
#include <array>
#include <algorithm>
#include <string_view>
#include <cstring>
#include <iostream>

namespace rk::util
{
    template<size_t N>
    struct FixedString {
        // 唯一的成员数据，确保内存布局紧凑
        // 初始化为全0，保证如果不满有默认的终止符
        std::array<char, N> buffer{};

        // =========================================================
        // 构造函数
        // =========================================================

        constexpr FixedString() = default;

        // 从 C 风格字符串构造 (处理了截断和空指针)
        constexpr FixedString(const char* str) {
            assign(str);
        }

        // 从 std::string_view 构造
        constexpr FixedString(std::string_view sv) {
            assign(sv);
        }

        constexpr FixedString(std::string str) {
            assign(str.c_str());
        }
        // =========================================================
        // 核心赋值逻辑
        // =========================================================

        constexpr void assign(std::string_view sv) {
            // 先清空（或者至少保证新内容之后是0），这里选择逐个填充
            // 如果追求极致性能，非 constexpr 环境可以用 memset/memcpy，但在 C++20 中 loop 也会被优化得很好
            size_t count = std::min(N, sv.size());
            for (size_t i = 0; i < count; ++i) {
                buffer[i] = sv[i];
            }
            // 如果原始字符串比 N 短，后面补零；如果长，截断且末尾可能无 \0
            // 为了安全，如果还有空间，手动补一个 \0 (可选策略，看具体协议需求)
            // 这里采用最原始策略：只拷贝，剩下的保持初始化时的0
            if (count < N) {
                for (size_t i = count; i < N; ++i) buffer[i] = '\0';
            }
        }

        constexpr void assign(const char* str) {
            if (!str) {
                *this = FixedString(); // 重置为0
                return;
            }
            // 手写循环以支持 constexpr strlen
            size_t i = 0;
            for (; i < N && str[i] != '\0'; ++i) {
                buffer[i] = str[i];
            }
            // 填充剩余部分为 0
            for (; i < N; ++i) {
                buffer[i] = '\0';
            }
        }

        // =========================================================
        // 赋值操作符
        // =========================================================

        constexpr FixedString& operator=(const char* str) {
            assign(str);
            return *this;
        }

        constexpr FixedString& operator=(std::string_view sv) {
            assign(sv);
            return *this;
        }

        // =========================================================
        // 访问与转换接口
        // =========================================================

        // 转为 string_view，这是现代 C++ 交互的核心接口
        // 注意：由于 buffer 可能不以 \0 结尾（如果填满了），我们需要检测实际长度
        constexpr std::string_view view() const {
            // 寻找第一个 \0，或者取 N
            // 在 C++20 constexpr 中不能用 memchr，手写查找
            size_t len = 0;
            for (; len < N; ++len) {
                if (buffer[len] == '\0') break;
            }
            return std::string_view(buffer.data(), len);
        }

        // 隐式转换为 string_view，方便直接传参
        constexpr operator std::string_view() const {
            return view();
        }

        // 获取原始指针 (慎用，不一定有 \0 结尾)
        constexpr const char* data() const { return buffer.data(); }
        constexpr const char* c_str() const { return buffer.data(); }
        constexpr char* data() { return buffer.data(); }


        constexpr size_t size() const { return view().size(); }
        constexpr size_t capacity() const { return N; }
        constexpr bool empty() const { return buffer[0] == '\0'; }

        // =========================================================
        // 比较操作符 (C++20 spaceship operator)
        // =========================================================

        // 支持与另一个 FixedString 比较
        auto operator<(const FixedString& other) const
        {
            return view() < other.view();
        }
        auto operator==(const FixedString& other) const
        {
            return view() == other.view();
        }
        // 支持与 const char* 比较
        constexpr bool operator==(const char* other) const {
            return view() == other;
        }

        // 支持与 string_view 比较

        std::string to_string() const {
            return data();
        }

    };
}