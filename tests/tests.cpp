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
#include <iostream>
#include <iocpp.h>

// IOC Example : Property Source Injeciton

class IProperty
{
public:
	virtual ~IProperty() = default;
	virtual std::string get(std::string key) const = 0;
};

class Property : public IProperty
{
public:
	std::string get(std::string key) const override
	{
		// Any property source applied here.
		return "Hello, " + key;
	}
};

class Application : public RESOURCE2(Property, IProperty, "myProps") /*Inject Property into Application*/
{
public:
	Application()
	{
		std::cout << "injected " << (intptr_t)RESOURCE2_THIS(Property, IProperty, "myProps") << std::endl;
	}
};

class Service : public AUTOWIRE(IProperty) /*AutoWire Property into Service*/
{
public:
	Service()
	{
		std::cout << "autowire " << (intptr_t)AUTOWIRE_THIS(IProperty) << std::endl;

		auto myValue = AUTOWIRE_THIS(IProperty)->get("myKey");
	}
};

int main()
{
	Application app;
	Service service1, service2;
	return 0;
}