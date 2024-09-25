#pragma once

#include "Crunch/Crunch.hpp"

namespace Cr
{
    template<typename T>
    class Singleton
    {
        public:
            Singleton() = delete;
            Singleton(const Singleton& other) = delete;
            Singleton(Singleton&& other) = delete;

            template<typename... Args>
            static void create(Args&&... args)
            {
                m_instance = std::make_unique<T>(std::forward<decltype(args)>(args)...);
            }

            static T& get()
            {
                return *m_instance;
            }

        private:
            static Unique<T> m_instance;
    };
}
