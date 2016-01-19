#pragma once


class KnProcFilterManager final
{
private:
	KnProcFilterManager(void);
	KnProcFilterManager(const KnProcFilterManager&);
	KnProcFilterManager& operator=(const KnProcFilterManager&);

public:
	~KnProcFilterManager(void);

public:
	static KnProcFilterManager& GetInstance(void);

private:
	static std::tr1::shared_ptr<KnProcFilterManager> _Instance;

private:
	HMODULE	m_dll;
};


#define MANAGER	(KnProcFilterManager::GetInstance())
