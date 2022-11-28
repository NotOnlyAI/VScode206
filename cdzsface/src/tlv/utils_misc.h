
#ifndef __UTILS_MISC_H__
#define __UTILS_MISC_H__

#include <mutex>
#include <memory>
#include <vector>
#include <functional>

// 通用宏
#define DEFAULT_CHN_ID (101)
#define CIF_WIDTH_             (352)
#define CIF_HEIGHT_            (288)

// 人脸处理分辨率
#define FACE_DL_WIDTH          (1280)
#define FACE_DL_HEIGHT         (720)

#define META_WIDTH_SIZE_SCALE  (10000)  // 元数据尺寸数值以0-10000的比例值发送
#define META_HEIGHT_SIZE_SCALE (10000)



template<typename T>
class Singleton 
{
public:
    static T * GetInstance(void)
    {
        static T m_Instance;
        return &m_Instance;
    }

protected:
    Singleton(){}
    virtual ~Singleton(){}
private:
    Singleton(const Singleton & rhs){}             // 复制构造
    Singleton& operator =(const Singleton & rhs){} // 赋值
};

namespace common {
template <typename T>
class Singleton {
public:
    template<typename... Args>
    static inline std::shared_ptr<T> instance(Args&& ... args) {
        // using double-check-lock.
        if (!instance_.get()) {
            std::unique_lock<std::mutex> _(instanceMutex_);
            if (!instance_.get()) {
                instance_.reset(new T(std::forward<Args>(args)...));
            }
        }
        return instance_;
    }

private:
    Singleton() = default;
    virtual ~Singleton() = default;
    Singleton(const Singleton&) = default;
    Singleton& operator = (const Singleton &) = delete;

    // instance
    static std::shared_ptr<T> instance_;
    static std::mutex instanceMutex_;
};
}  // namespace common

template <typename T>
std::shared_ptr<T> common::Singleton<T>::instance_;

template <typename T>
std::mutex common::Singleton<T>::instanceMutex_;

#define SINGLETON_DECL(type) \
    friend class std::shared_ptr< type >; \
    friend class common::Singleton< type >;


// C++11 资源池实现
namespace common {
template < typename T >
class BaseObjectPool {
public:
    BaseObjectPool(){}
    ~BaseObjectPool(){
        empty();
    }
    using DeleterType = std::function<void(T*)>;
    virtual void add(std::unique_ptr<T> t) {
        std::unique_lock<std::mutex> _(mutex_);
        pool_.push_back(std::move(t));
        total_++;
    }

    virtual void sub() {
        if (pool_.empty()) {
            return;
        }

        std::unique_lock<std::mutex> _(mutex_);
        if (pool_.empty()) {
            return;
        }
        pool_.pop_back();
        total_--;
    }

    //must check first
    virtual std::unique_ptr<T, DeleterType> get() {
        if (pool_.empty()) {
            return nullptr;
        }

        std::unique_lock<std::mutex> _(mutex_);
        if (pool_.empty()) {
            return nullptr;
        }
  
        std::unique_ptr<T, DeleterType> ptr(pool_.back().release(), [this](T* t) {
            std::unique_lock<std::mutex> mtx(mutex_);
            pool_.push_back(std::unique_ptr<T>(t));
        });

        pool_.pop_back();
        return std::move(ptr);
    }

    //must check first
    virtual std::shared_ptr<T> get_shared() {
        if (pool_.empty()) {
            return nullptr;
        }

        std::unique_lock<std::mutex> _(mutex_);
        if (pool_.empty()) {
            return nullptr;
        }
        auto pin = std::unique_ptr<T>(std::move(pool_.back()));
        pool_.pop_back();
        //release返回 t 再组成unique_ptr的构造
        return std::shared_ptr<T>(pin.release(), [this](T* t) {
            std::unique_lock<std::mutex> mtx(mutex_);
            pool_.push_back(std::unique_ptr<T>(t));
        });
    }

    virtual bool empty() const {
        return pool_.empty();
    }

    virtual size_t size() {
        std::unique_lock<std::mutex> _(mutex_);
        return pool_.size();
    }

    virtual size_t total() const {
        //std::unique_lock<std::mutex> _(mutex_);
        return total_;
    }

protected:
    size_t total_ = 0;

    std::vector<std::unique_ptr<T>> pool_;
    // lock
    std::mutex mutex_;
};
}  // namespace common


// 资源池封装
template<typename T>
class ObjPool {
public:
	ObjPool() {
	}
	~ObjPool() {
		Empty();
	}

	template<typename... Args>
	int32_t AddMemory(Args&&... value) {
		std::unique_ptr<T> buf(new (std::nothrow) T(value...));
		if (buf) {
			bufPool_.add(std::move(buf));
			return 0;
		}
		return -1;
	}

    std::shared_ptr<T> GetObj()  {
		return bufPool_.get_shared();
	}

    size_t size() {
		return bufPool_.size();
	}

    size_t total() {
        return bufPool_.total();
    }

private:
	void Empty() {
		if (0 != bufPool_.total()) {
			bufPool_.sub();
		}
	}

private:
	common::BaseObjectPool<T> bufPool_{};
};



#endif /* __UTILS_MISC_H__ */
