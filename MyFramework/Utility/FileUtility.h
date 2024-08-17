#pragma once
#include <string>
#include <vector>
#include <memory>

class ObserverBase
{
public: 
	virtual void Update() = 0;
};

class FileDiffObserver : public ObserverBase
{
public:
	virtual void Update() = 0;

	void SetupPath(const std::wstring& pathName) { m_pathName = pathName; }
	std::wstring GetPath() const { return m_pathName; }
	bool IsDiffDetected() const { return m_isDiffDetected; }
protected:
	std::wstring m_pathName;
	bool m_isDiffDetected;
};

class HlslDiffObserver : public FileDiffObserver
{
public:
	virtual void Update() override;

protected:
	std::string m_prevShader;
	std::string m_currentShader;
};

class DiffNotifier
{
public:
	void AddObserver(std::unique_ptr<FileDiffObserver>&& observer) { m_observerList.emplace_back(std::move(observer)); }

	template<class T>
	void AddObserver(const std::wstring& path)
	{
		static_assert(std::is_base_of<FileDiffObserver, T>::value == true);
		
		std::unique_ptr<T> observer = std::make_unique<T>();
		observer->SetupPath(path);
		m_observerList.emplace_back(std::move(observer));
	}

	void Execute();

	// ¬Œ÷‚µ‚½Å‰‚ÌDiff‚ğæ“¾‚·‚é
	std::wstring GetFirstObserverPath() const;

protected:
	std::vector<std::unique_ptr<FileDiffObserver>> m_observerList;
};