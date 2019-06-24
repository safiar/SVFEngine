// ----------------------------------------------------------------------- //
//
// MODULE  : vecdata.h
//
// PURPOSE : Оболочки управления массивами указателей
//
// CREATED : SavF. ⚡ Savenkov Filipp A. (2018)
//
// ----------------------------------------------------------------------- //

#ifndef _VECDATA_HELPER_H
#define _VECDATA_HELPER_H

#include "types.h"

// ??? проверки касательно конвертации uint64 в size_t для #ifdef BUILDx32 ??
// malloc ERRORs допилить

namespace SAVFGAME
{
	/*
	template <class T>
	struct DATAP // Вспомогательная оболочка к shared_ptr
	{
		DATAP() {};
		DATAP(bool create)            { if (create) Create(); };
		DATAP(std::shared_ptr<T> ptr) { data = ptr; };
		/////////
		T * get() { return data.get(); }
		/////////
		//>> <true> если есть *data ; <false> иначе
		bool Check()
		{
			if (data == nullptr) return false;
			return true;
		}
		//>> исполняет data->Close()
		void Close(bool delete_data)
		{
			if (data != nullptr) data->Close();
			if (delete_data) Delete();
		}
		//>> исполняет data = nullptr
		void Delete()
		{
			data = nullptr;
		}
		//>> исполняет data = make_shared <T> () , но только если data == nullptr
		bool Create()
		{
			if (data != nullptr) return false;
			data = std::make_shared <T> ();
			return true;
		}
		//>> delete + create 
		void ReCreate(bool close_data)
		{
			if (close_data) Close(1);
			else Delete();
			data = std::make_shared <T> ();
		}
		////
		                T* operator->() { return data.get(); }
		std::shared_ptr<T> operator()() { return data; }
		bool operator==(std::shared_ptr<T> ptr) { return (data == ptr); }
		bool operator!=(std::shared_ptr<T> ptr) { return (data != ptr); }
		////
		std::shared_ptr<T> data;
	};

	template <class T>
	struct VECDATAP // Вспомогательная оболочка для управления массивом shared_ptr
	{
		std::vector<DATAP<T>> v;

		VECDATAP()  { };
		~VECDATAP() { Delete(1); };
		size_t size()              { return v.size(); }
		void emplace_back()        { Add(1); }
		void emplace_back_create() { AddCreate(1); }
		void pop_back()            { v.pop_back(); }
		void reserve(size_t count) { v.reserve(count); }
		DATAP<T>& last()           { return v[v.size()-1]; }
		size_t last_id()           { return   v.size()-1 ; }

		//>> Сбрасывает указатели в nullptr
		void Delete(bool erase_data)
		{
			// Deleting
			for (auto & i : v) i.Delete();
			if (erase_data)
				v.erase(v.begin(), v.end());
		}
		//>> Сбрасывает указатели в nullptr
		void Delete(bool erase_data, size_t from_id)
		{
			size_t i, size = v.size();	if (!size) return;
										if (from_id >= size) return;
			// Deleting
			for (i=from_id; i<size; i++) v[i].Delete();
			if (erase_data)
				v.erase(v.begin() + from_id, v.end());
		}
		//>> Сбрасывает указатели в nullptr
		void Delete(bool erase_data, size_t from_id, size_t to_id)
		{
			size_t i, size = v.size();	if (!size) return;
										if (from_id >= size || to_id > size || to_id <= from_id) return;
			// Deleting
			for (i=from_id; i<to_id; i++) v[i].Delete();
			if (erase_data)
				v.erase(v.begin() + from_id, v.end() + to_id);
		}
		//>> Задаёт новый вектор в count мест (все указатели пустые)
		void New(size_t count)
		{
			// Deleting
			Delete(1);

			// Reserving
			v.reserve(count);
			for (size_t i = 0; i<count; i++) v.emplace_back(DATAP<T>());
		}
		//>> Задаёт новый вектор в count мест (все указатели ведут к новым объектам) :: abstract classes restricted via this function
		void Create(size_t count)
		{
			// Deleting + Reserving
			New(count);

			// Creating
			for (auto & i : v) i.Create();
		}		
		//>> Вызывает фактическое удаления данных объекта (производит очистку)
		void Close(bool delete_data, bool erase_data)
		{
			// Closing
			for (auto & i : v) i.Close(0);
			if (delete_data)
				Delete(erase_data);
		}
		//>> Вызывает фактическое удаления данных объекта (производит очистку)
		void Close(bool delete_data, bool erase_data, size_t from_id)
		{
			size_t i, size = v.size();	if (!size) return;
										if (from_id >= size) return;
			// Closing
			for (i=from_id; i<size; i++) v[i].Close(0);
			if (delete_data)
				Delete(erase_data, from_id);
		}
		//>> Вызывает фактическое удаления данных объекта (производит очистку)
		void Close(bool delete_data, bool erase_data, size_t from_id, size_t to_id)
		{
			size_t i, size = v.size();	if (!size) return;
										if (from_id >= size || to_id > size || to_id <= from_id) return;
			// Closing
			for (i=from_id; i<to_id; i++) v[i].Close(0);
			if (delete_data)
				Delete(erase_data, from_id, to_id);
		}
		//>> Добавляет count новых мест в конец (указатели пустые)
		void Add(size_t count)
		{
			for (size_t i=0; i<count; i++) v.emplace_back(DATAP<T>());
		}
		//>> Добавляет count новых мест в конец (указатели ведут к новым объектам) :: abstract classes restricted via this function
		void AddCreate(size_t count)
		{	
			size_t size   = v.size();
			size_t target = size + count;
			Add(count);
			for (size_t i = size; i<target; i++)
				v[i].Create();
		}
		//>> Увеличивает до count новых мест (указатели пустые)
		void Set(size_t count)
		{
			size_t size = v.size();
			if (count > size) Add(count - size);
		}
		//>> Увеличивает до count новых мест (указатели ведут к новым объектам) :: abstract classes restricted via this function
		void SetCreate(size_t count)
		{
			size_t size = v.size();
			if (count > size) AddCreate(count - size);
		}

		      DATAP<T>& operator[](size_t idx)       { return v[idx]; }
		const DATAP<T>& operator[](size_t idx) const { return v[idx]; }
		void operator()(size_t num_to_create)  { Create(num_to_create); }
	};
//*/

	struct VECDATA_PROTOTYPE
	{
	protected:
		// vector data
	public:
		// simple functions
		virtual uint64 size() = 0;
		virtual void emplace_back() = 0;
		virtual void emplace_back_create() = 0;
		virtual void pop_back() = 0;
		virtual void reserve(uint64 count) = 0;
	//	virtual T * last() = 0;
		virtual uint64 last_id() = 0;

	//	virtual std::vector<T>::iterator begin() = 0;
	//	virtual std::vector<T>::iterator end()   = 0;

		// main functions
		virtual void Delete(bool erase_data) = 0;
		virtual void Delete(bool erase_data, uint64 from_id) = 0;
		virtual void Delete(bool erase_data, uint64 from_id, uint64 to_id) = 0;
		virtual void New(uint64 count) = 0;
		virtual void Create(uint64 count) = 0;
		virtual void Close(bool delete_data, bool erase_data) = 0;
		virtual void Close(bool delete_data, bool erase_data, uint64 from_id) = 0;
		virtual void Close(bool delete_data, bool erase_data, uint64 from_id, uint64 to_id) = 0;
		virtual void Add(uint64 count) = 0;
		virtual void AddCreate(uint64 count) = 0;
		virtual void Set(uint64 count) = 0;
		virtual void SetCreate(uint64 count) = 0;
	//	virtual void Reset(T * ptr, uint64 id) = 0;
		virtual void ReCreate(uint64 id) = 0;
		virtual void Swap(uint64 id_0, uint64 id_1) = 0;
		
		// extra functions
	//	virtual T * operator[](uint64 idx) = 0;
		virtual void operator()(uint64 num_to_create) = 0;
	};

	template <class T>
	// Вспомогательная оболочка для управления массивом shared ptr
	struct VECDATAP
//	struct VECDATAP : public VECDATA_PROTOTYPE
	{
	protected:
		std::vector<std::shared_ptr<T>> v;
	public:
		VECDATAP() {}; //: VECDATA_PROTOTYPE() {};
		~VECDATAP() { Delete(1); };
		uint64 size()              { return (uint64) v.size(); }
		void emplace_back()        { Add(1); }
		void emplace_back_create() { AddCreate(1); }
		void pop_back()            { v.pop_back(); }
		void reserve(uint64 count) { v.reserve((size_t)count); }
		std::shared_ptr<T> last()  { return        v[v.size() - 1]; }
		uint64 last_id()           { return (uint64) v.size() - 1;  }

		auto begin() -> decltype(v.begin())
		{
			return v.begin();
		}
		auto end() -> decltype(v.end())
		{
			return v.end();
		}

		//>> Сбрасывает указатели в nullptr
		void Delete(bool erase_data)
		{
			// Deleting
			for (auto & i : v) i = nullptr;
			if (erase_data)
				v.erase(v.begin(), v.end());
		}
		//>> Сбрасывает указатели в nullptr
		void Delete(bool erase_data, uint64 from_id)
		{
			uint64 i, size = (uint64) v.size();		if (!size) return;
													if (from_id >= size) return;
			// Deleting
			for (i=from_id; i<size; i++) v[(size_t)i] = nullptr;
			if (erase_data)
				v.erase(v.begin() + (size_t) from_id, v.end());
		}
		//>> Сбрасывает указатели в nullptr
		void Delete(bool erase_data, uint64 from_id, uint64 to_id)
		{
			uint64 i, size = (uint64) v.size();		if (!size) return;
													if (from_id >= size || to_id > size || to_id <= from_id) return;
			// Deleting
			for (i=from_id; i<to_id; i++) v[(size_t)i] = nullptr;
			if (erase_data)
				v.erase(v.begin() + (size_t) from_id, v.begin() + (size_t) to_id);
		}
		//>> Задаёт новый вектор в count мест (все указатели пустые)
		void New(uint64 count)
		{
			// Deleting
			Delete(1);

			// Reserving
			v.reserve((size_t)count);
			for (uint64 i = 0; i<count; i++) v.emplace_back(nullptr);
		}
		//>> Задаёт новый вектор в count мест (все указатели ведут к новым объектам) :: abstract classes restricted via this function
		void Create(uint64 count)
		{
			// Deleting + Reserving
			New(count);

			// Creating
			for (auto & i : v) i = std::make_shared<T>();
		}		
		//>> Вызывает фактическое удаления данных объекта (производит очистку)
		void Close(bool delete_data, bool erase_data)
		{
			// Closing
			for (auto & i : v) if (i != nullptr) i->Close();
			if (delete_data)
				Delete(erase_data);
		}
		//>> Вызывает фактическое удаления данных объекта (производит очистку)
		void Close(bool delete_data, bool erase_data, uint64 from_id)
		{
			uint64 i, size = (uint64) v.size();		if (!size) return;
													if (from_id >= size) return;
			// Closing
			for (i=from_id; i<size; i++) if (v[(size_t)i] != nullptr) v[(size_t)i]->Close();
			if (delete_data)
				Delete(erase_data, from_id);
		}
		//>> Вызывает фактическое удаления данных объекта (производит очистку)
		void Close(bool delete_data, bool erase_data, uint64 from_id, uint64 to_id)
		{
			uint64 i, size = (uint64) v.size();		if (!size) return;
													if (from_id >= size || to_id > size || to_id <= from_id) return;
			// Closing
			for (i=from_id; i<to_id; i++) if (v[(size_t)i] != nullptr) v[(size_t)i]->Close();
			if (delete_data)
				Delete(erase_data, from_id, to_id);
		}
		//>> Добавляет count новых мест в конец (указатели пустые)
		void Add(uint64 count)
		{
			for (uint64 i=0; i<count; i++) v.emplace_back(nullptr);
		}
		//>> Добавляет count новых мест в конец (указатели ведут к новым объектам) :: abstract classes restricted via this function
		void AddCreate(uint64 count)
		{	
			uint64 size   = (uint64) v.size();
			uint64 target = size + count;
			Add(count);
			for (uint64 i=size; i<target; i++)
				v[(size_t)i] = std::make_shared<T>();
		}
		//>> Увеличивает до count новых мест (указатели пустые)
		void Set(uint64 count)
		{
			uint64 size = v.size();
			if (count > size) Add(count - size);
		}
		//>> Увеличивает до count новых мест (указатели ведут к новым объектам) :: abstract classes restricted via this function
		void SetCreate(uint64 count)
		{
			uint64 size = v.size();
			if (count > size) AddCreate(count - size);
		}
		//>> Устанавливает указатель в позицию (другой объект будет удалён)
		void Reset(std::shared_ptr<T> ptr, uint64 id)
		{
			if (id >= (uint64) v.size()) { _MBM(ERROR_IDLIMIT); return; }

			v[(size_t)id] = ptr;
		}
		//>> Пересоздаёт объект на позиции (другой объект будет удалён) :: abstract classes restricted via this function
		void ReCreate(uint64 id)
		{
			if (id >= (uint64) v.size()) { _MBM(ERROR_IDLIMIT); return; }

			v[(size_t)id] = std::make_shared<T>();
		}
		//>> Меняет местами 2 указателя
		void Swap(uint64 id_0, uint64 id_1)
		{
			uint64 size = (uint64) v.size();
			if (id_0 >= size || id_1 >= size) { _MBM(ERROR_IDLIMIT); return; }

			_SWAP(v[id_0], v[id_1]);
		}

		std::shared_ptr<T> operator[](uint64 idx)  { return v[(size_t)idx]; }
		void operator()(uint64 num_to_create) { Create(num_to_create); }
	};

	template <class T>
	// Вспомогательная оболочка для гибридного управления массивом обычных + shared ptr
	struct VECDATAPP
//	struct VECDATAPP : public VECDATA_PROTOTYPE
	{
	protected:
		std::vector<std::shared_ptr<T>> vs;
		std::vector<T*>                 v;
	public:
		VECDATAPP() {}; //: VECDATA_PROTOTYPE() {};
		~VECDATAPP() { Delete(1); };
		uint64 size()              { return (uint64) vs.size(); }
		void emplace_back()        { Add(1); }
		void emplace_back_create() { AddCreate(1); }
		void pop_back()            { v.pop_back();     vs.pop_back();     }
		void reserve(uint64 count) { v.reserve((size_t)count); vs.reserve((size_t)count); }
		T *    last()              { return        v[vs.size() - 1]; }
		uint64 last_id()           { return (uint64) vs.size() - 1;  }

		auto begin() -> decltype(v.begin())
		{
			return v.begin();
		}
		auto end() -> decltype(v.end())
		{
			return v.end();
		}
		
		//>> Сбрасывает указатели в nullptr
		void Delete(bool erase_data)
		{
			size_t i, size = vs.size();

			// Deleting
			for (i = 0; i < size; i++)	{
				vs[i] = nullptr;
				v[i]  = nullptr;		}			
			if (erase_data)								{
				vs.erase ( vs.begin(), vs.end() );
				v.erase  ( v.begin(),  v.end()  );		}
		}
		//>> Сбрасывает указатели в nullptr
		void Delete(bool erase_data, uint64 from_id)
		{
			uint64 i, size = (uint64) vs.size();	if (!size) return;
			if (from_id >= size) return;

			// Deleting
			for (i = from_id; i < size; i++)	{
				vs[i] = nullptr;
				v[i]  = nullptr;				}
			if (erase_data)										{
				vs.erase ( vs.begin() + from_id, vs.end() );
				v.erase  ( v.begin()  + from_id, v.end()  );	}
		}
		//>> Сбрасывает указатели в nullptr
		void Delete(bool erase_data, uint64 from_id, uint64 to_id)
		{
			uint64 i, size = (uint64) vs.size();		  if (!size) return;
			if (from_id >= size || to_id > size || to_id <= from_id) return;
			
			// Deleting
			for (i = from_id; i < to_id; i++)	{
				vs[(size_t)i] = nullptr;
				v[(size_t)i]  = nullptr;		}
			if (erase_data)																	{
				vs.erase ( vs.begin() + (size_t) from_id, vs.begin() + (size_t) to_id );
				v.erase  (  v.begin() + (size_t) from_id,  v.begin() + (size_t) to_id );	}
		}
		//>> Задаёт новый вектор в count мест (все указатели пустые)
		void New(uint64 count)
		{
			// Deleting
			Delete(1);

			// Reserving
			vs.reserve((size_t)count);
			v.reserve((size_t)count);
			for (uint64 i = 0; i < count; i++)
			{
				vs.emplace_back(nullptr);
				v.emplace_back(nullptr);
			}
		}
		//>> Задаёт новый вектор в count мест (все указатели ведут к новым объектам) :: abstract classes restricted via this function
		void Create(uint64 count)
		{
			// Deleting + Reserving
			New(count);

			// Creating
			uint64 i, size = (uint64) vs.size();
			for (i=0; i<size; i++)
			{
				vs[(size_t)i] = std::make_shared<T>();
				v[(size_t)i] = vs[(size_t)i].get();
			}
		}
		//>> Вызывает фактическое удаления данных объекта (производит очистку)
		void Close(bool delete_data, bool erase_data)
		{
			// Closing
			for (auto & i : v) if (i != nullptr) i->Close();
			if (delete_data)
				Delete(erase_data);
		}
		//>> Вызывает фактическое удаления данных объекта (производит очистку)
		void Close(bool delete_data, bool erase_data, uint64 from_id)
		{
			uint64 i, size = (uint64) vs.size();			if (!size) return;
			if (from_id >= size) return;
			// Closing
			for (i = from_id; i<size; i++) if (v[(size_t)i] != nullptr) v[(size_t)i]->Close();
			if (delete_data)
				Delete(erase_data, from_id);
		}
		//>> Вызывает фактическое удаления данных объекта (производит очистку)
		void Close(bool delete_data, bool erase_data, uint64 from_id, uint64 to_id)
		{
			uint64 i, size = (uint64) vs.size();		  if (!size) return;
			if (from_id >= size || to_id > size || to_id <= from_id) return;
			// Closing
			for (i = from_id; i<to_id; i++) if (v[(size_t)i] != nullptr) v[(size_t)i]->Close();
			if (delete_data)
				Delete(erase_data, from_id, to_id);
		}
		//>> Добавляет count новых мест в конец (указатели пустые)
		void Add(uint64 count)
		{
			for (uint64 i = 0; i < count; i++)
			{
				vs.emplace_back(nullptr);
				v.emplace_back(nullptr);
			}
		}
		//>> Добавляет count новых мест в конец (указатели ведут к новым объектам) :: abstract classes restricted via this function
		void AddCreate(uint64 count)
		{
			uint64 size   = (uint64) vs.size();
			uint64 target = size + count;
			Add(count);
			for (uint64 i = size; i<target; i++)
			{
				vs[(size_t)i] = std::make_shared<T>();
				v[(size_t)i] = vs[(size_t)i].get();
			}
		}
		//>> Увеличивает до count новых мест (указатели пустые)
		void Set(uint64 count)
		{
			uint64 size = (uint64) vs.size();
			if (count > size) Add(count - size);
		}
		//>> Увеличивает до count новых мест (указатели ведут к новым объектам) :: abstract classes restricted via this function
		void SetCreate(uint64 count)
		{
			uint64 size = vs.size();
			if (count > size) AddCreate(count - size);
		}
		//>> Устанавливает указатель в позицию (другой объект будет удалён)
		void Reset(std::shared_ptr<T> ptr, uint64 id)
		{
			if (id >= (uint64) vs.size()) { _MBM(ERROR_IDLIMIT); return; }

			vs[(size_t)id] = ptr;
			v [(size_t)id] = ptr.get();
		}
		//>> Пересоздаёт объект на позиции (другой объект будет удалён) :: abstract classes restricted via this function
		void ReCreate(uint64 id)
		{
			if (id >= (uint64) vs.size()) { _MBM(ERROR_IDLIMIT); return; }

			vs[(size_t)id] = std::make_shared<T>();
			v [(size_t)id] = vs[(size_t)id].get();
		}
		//>> Меняет местами 2 указателя
		void Swap(uint64 id_0, uint64 id_1)
		{
			uint64 size = (uint64) vs.size();
			if (id_0 >= size || id_1 >= size) { _MBM(ERROR_IDLIMIT); return; }

			_SWAP(vs[(size_t)id_0], vs[(size_t)id_1]);
			_SWAP(v [(size_t)id_0], v [(size_t)id_1]);
		}

		T *  operator[](uint64 idx)           { return v[(size_t)idx]; }
		void operator()(uint64 num_to_create) { Create(num_to_create); }

		std::shared_ptr<T> shared(uint64 idx) { return vs[(size_t)idx]; }
	};

	template <class T>
	// Вспомогательная оболочка для управления массивом обычных указателей
	struct VECPDATA
//	struct VECPDATA : public VECDATA_PROTOTYPE
	{
	protected:
		std::vector<T*> v;
	public:
		VECPDATA() {}; //: VECDATA_PROTOTYPE() {};
		~VECPDATA() { Delete(1); };
		uint64 size()              { return (uint64) v.size(); }
		void emplace_back()        { Add(1); }
		void emplace_back_create() { AddCreate(1); }
		void pop_back()            { v.pop_back(); }
		void reserve(uint64 count) { v.reserve((size_t)count); }
		T *    last()              { return        v[v.size() - 1]; }
		uint64 last_id()           { return (uint64) v.size() - 1;  }

		auto begin() -> decltype(v.begin())
		{
			return v.begin();
		}
		auto end() -> decltype(v.end())
		{
			return v.end();
		}

		//>> Удаляет объекты и сбрасывает указатели в nullptr
		void Delete(bool erase_data)
		{
			// Deleting
			for (auto & i : v) _DELETE(i);
			if (erase_data)
				v.erase(v.begin(), v.end());
		}
		//>> Удаляет объекты и сбрасывает указатели в nullptr
		void Delete(bool erase_data, uint64 from_id)
		{
			uint64 i, size = (uint64) v.size();		if (!size) return;
			if (from_id >= size) return;
			// Deleting
			for (i = from_id; i<size; i++) _DELETE(v[i]);
			if (erase_data)
				v.erase(v.begin() + from_id, v.end());
		}
		//>> Удаляет объекты и сбрасывает указатели в nullptr
		void Delete(bool erase_data, uint64 from_id, uint64 to_id)
		{
			uint64 i, size = (uint64) v.size();			  if (!size) return;
			if (from_id >= size || to_id > size || to_id <= from_id) return;
			// Deleting
			for (i = from_id; i<to_id; i++) _DELETE(v[(size_t)i]);
			if (erase_data)
				v.erase(v.begin() + (size_t) from_id, v.begin() + (size_t) to_id);
		}
		//>> Задаёт новый вектор в count мест (все указатели пустые)
		void New(uint64 count)
		{
			// Deleting
			Delete(1);

			// Reserving
			v.reserve((size_t)count);
			for (uint64 i = 0; i<count; i++) v.emplace_back(nullptr);
		}
		//>> Задаёт новый вектор в count мест (все указатели ведут к новым объектам) :: abstract classes restricted via this function
		void Create(uint64 count)
		{
			// Deleting + Reserving
			New(count);

			// Creating
			for (auto & i : v) i = new T;
		}
		//>> Вызывает метод закрытия объекта
		void Close(bool delete_data, bool erase_data)
		{
			// Closing
			for (auto & i : v) if (i != nullptr) i->Close();
			if (delete_data)
				Delete(erase_data);
		}
		//>> Вызывает метод закрытия объекта
		void Close(bool delete_data, bool erase_data, uint64 from_id)
		{
			uint64 i, size = (uint64) v.size();		if (!size) return;
			if (from_id >= size) return;
			// Closing
			for (i = from_id; i<size; i++) if (v[(size_t)i] != nullptr) v[(size_t)i]->Close();
			if (delete_data)
				Delete(erase_data, from_id);
		}
		//>> Вызывает метод закрытия объекта
		void Close(bool delete_data, bool erase_data, uint64 from_id, uint64 to_id)
		{
			uint64 i, size = (uint64) v.size();			  if (!size) return;
			if (from_id >= size || to_id > size || to_id <= from_id) return;
			// Closing
			for (i = from_id; i<to_id; i++) if (v[(size_t)i] != nullptr) v[(size_t)i]->Close();
			if (delete_data)
				Delete(erase_data, from_id, to_id);
		}
		//>> Добавляет count новых мест в конец (указатели пустые)
		void Add(uint64 count)
		{
			for (uint64 i = 0; i<count; i++) v.emplace_back(nullptr);
		}
		//>> Добавляет count новых мест в конец (указатели ведут к новым объектам) :: abstract classes restricted via this function
		void AddCreate(uint64 count)
		{
			uint64 size   = (uint64) v.size();
			uint64 target = size + count;
			Add(count);
			for (uint64 i = size; i<target; i++)
				v[(size_t)i] = new T;
		}
		//>> Увеличивает до count новых мест (указатели пустые)
		void Set(uint64 count)
		{
			uint64 size = (uint64) v.size();
			if (count > size) Add(count - size);
		}
		//>> Увеличивает до count новых мест (указатели ведут к новым объектам) :: abstract classes restricted via this function
		void SetCreate(uint64 count)
		{
			uint64 size = (uint64) v.size();
			if (count > size) AddCreate(count - size);
		}
		//>> Устанавливает указатель в позицию (другой объект будет удалён)
		void Reset(T * ptr, uint64 id)
		{
			if (id >= (uint64) v.size()) { _MBM(ERROR_IDLIMIT); return; }

			_DELETE(v[(size_t)id]);
			v[(size_t)id] = ptr;
		}
		//>> Пересоздаёт объект на позиции (другой объект будет удалён) :: abstract classes restricted via this function
		void ReCreate(uint64 id)
		{
			if (id >= (uint64)v.size()) { _MBM(ERROR_IDLIMIT); return; }

			_DELETE(v[(size_t)id]);
			v[(size_t)id] = new T;
		}
		//>> Меняет местами 2 указателя
		void Swap(uint64 id_0, uint64 id_1)
		{
			uint64 size = (uint64) v.size();
			if (id_0 >= size || id_1 >= size) { _MBM(ERROR_IDLIMIT); return; }

			_SWAP(v[id_0], v[id_1]);
		}

		T *  operator[](uint64 idx)            { return v[(size_t)idx]; }
		void operator()(uint64 num_to_create)  { Create(num_to_create); }
	};

}

#endif  // _VECDATA_HELPER_H