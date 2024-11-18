#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include <any>
#include <map>
#include <memory>
#include <string>
#include <cstdint>
#include <functional>

namespace iocpp
{
	inline constexpr uint32_t Hash(const char* value)
	{
		uint32_t hash = 0; // From JDK 8
		if (value == nullptr) return hash;
		while (*value) hash = hash * 31 + (*value++);
		return hash;
	}

	inline constexpr uint32_t Hash(std::string const& value)
	{
		return Hash(value.c_str());
	}

	class Bean
	{
	public:
		std::any Data;
		std::function<void(std::any& src, std::any& dst)> Cast;
	};

	class BeanManager
	{
	public:
		static BeanManager& Instance();

		template<class T, class I, std::enable_if_t<std::is_same_v<T, I>, int> = 0>
		std::weak_ptr<std::shared_ptr<Bean>> create(uint32_t N = 0)
		{
			auto name = ((uint64_t)Hash(typeid(T).name()) << 32) | N;
			auto result = m_Beans[name];
			if (result == nullptr)
			{
				result = m_Beans[name] = std::make_shared<std::shared_ptr<Bean>>();
			}
			if (result->get() == nullptr)
			{
				auto bean = std::make_shared<Bean>();
				bean->Data = std::make_shared<T>();
				bean->Cast = [](std::any& src, std::any& dst) { dst = src; };
				(*result) = bean;
			}
			return result;
		}

		template<class T, class I, std::enable_if_t<std::is_same_v<T, I> == false && std::is_base_of_v<I, T>, int> = 0>
		std::weak_ptr<std::shared_ptr<Bean>> create(uint32_t N = 0)
		{
			auto name1 = ((uint64_t)Hash(typeid(I).name()) << 32) | N;
			auto name2 = ((uint64_t)Hash(typeid(T).name()) << 32) | N;
			auto name3 = ((uint64_t)Hash(typeid(I).name()) << 32) | 0;
			auto result = m_Beans[name1];
			if (result == nullptr)
			{
				result = m_Beans[name1] = m_Beans[name2] = std::make_shared<std::shared_ptr<Bean>>();
				m_Beans.emplace(name3, result);
			}
			if (result->get() == nullptr)
			{
				auto bean = std::make_shared<Bean>();
				bean->Data = std::make_shared<T>();
				bean->Cast = [](std::any& src, std::any& dst) { dst = std::dynamic_pointer_cast<I>(std::any_cast<std::shared_ptr<T>>(src)); };
				(*result) = bean;
				if (N) (*m_Beans[name3]) = (*result);
			}
			return result;
		}

		template<class T>
		std::weak_ptr<std::shared_ptr<Bean>> depend(uint32_t N = 0)
		{
			auto name = ((uint64_t)Hash(typeid(T).name()) << 32) | N;
			auto result = m_Beans[name];
			if (result == nullptr)
			{
				result = m_Beans[name] = std::make_shared<std::shared_ptr<Bean>>();
			}
			return result;
		}

	public:
		std::map<uint64_t, std::shared_ptr<std::shared_ptr<Bean>>> m_Beans;
	};

	template<class T, class I, uint32_t N = 0>
	class Resource
	{
	public:
		Resource()
		{
			m_Bean = BeanManager::Instance().create<T, I>(N);
		}

	protected:
		T* bean() const
		{
			if (m_Bean.lock() == nullptr) return nullptr;
			if (m_Bean.lock()->get() == nullptr) return nullptr;
			auto result = std::any_cast<std::shared_ptr<T>> (m_Bean.lock()->get()->Data);
			return result.get();
		}

	private:
		std::weak_ptr<std::shared_ptr<Bean>> m_Bean;
	};

	template<class T, uint32_t N = 0>
	class Autowire
	{
	public:
		Autowire()
		{
			m_Bean = BeanManager::Instance().depend<T>(N);
		}

	protected:
		T* bean() const
		{
			std::any dst;
			if (m_Bean.lock() == nullptr) return nullptr;
			if (m_Bean.lock()->get() == nullptr) return nullptr;
			m_Bean.lock()->get()->Cast(m_Bean.lock()->get()->Data, dst);
			auto result = std::any_cast<std::shared_ptr<T>> (dst);
			return result.get();
		}

	private:
		std::weak_ptr<std::shared_ptr<Bean>> m_Bean;
	};

	template<class T, class I>
	class ResourceThis
	{
	public:
		ResourceThis(uint32_t N = 0)
		{
			m_Bean = BeanManager::Instance().create<T, I>(N);
		}

		T* bean() const
		{
			if (m_Bean.lock() == nullptr) return nullptr;
			if (m_Bean.lock()->get() == nullptr) return nullptr;
			auto result = std::any_cast<std::shared_ptr<T>> (m_Bean.lock()->get()->Data);
			return result.get();
		}

	private:
		std::weak_ptr<std::shared_ptr<Bean>> m_Bean;
	};

	template<class T>
	class AutowireThis
	{
	public:
		AutowireThis(uint32_t N = 0)
		{
			m_Bean = BeanManager::Instance().depend<T>(N);
		}

		T* bean() const
		{
			std::any dst;
			if (m_Bean.lock() == nullptr) return nullptr;
			if (m_Bean.lock()->get() == nullptr) return nullptr;
			m_Bean.lock()->get()->Cast(m_Bean.lock()->get()->Data, dst);
			auto result = std::any_cast<std::shared_ptr<T>> (dst);
			return result.get();
		}

	private:
		std::weak_ptr<std::shared_ptr<Bean>> m_Bean;
	};
}

// Autowire(Type or Interface of Type)
#define AUTOWIRE(T) iocpp::Autowire<T, 0>
// Autowire(Type or Interface of Type, Name)
#define AUTOWIRE2(T, N) iocpp::Autowire<T, iocpp::Hash(N)>
// Autowire(Type or Interface of Type)
#define AUTOWIRE_DATA(T) iocpp::AutowireThis<T>(0).bean()
// Autowire(Type or Interface of Type, Name)
#define AUTOWIRE2_DATA(T, N) iocpp::AutowireThis<T>(iocpp::Hash(N)).bean()

// Resource(Type, Name)
#define RESOURCE(T, N) iocpp::Resource<T, T, iocpp::Hash(N)>
// Resource(Type, Interface of Type, Name)
#define RESOURCE2(T, I, N) iocpp::Resource<T, I, iocpp::Hash(N)>
// Resource(Type, Name)
#define RESOURCE_DATA(T, N) iocpp::ResourceThis<T, T>(iocpp::Hash(N)).bean()
// Resource(Type, Interface of Type, Name)
#define RESOURCE2_DATA(T, I, N) iocpp::ResourceThis<T, I>(iocpp::Hash(N)).bean()