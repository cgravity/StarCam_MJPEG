#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <string>
#include <memory>

#include <windows.h>

class SaveThread;

// SaveBuffers are passed back and forth between capture and I/O threads. 
// They are owned by one thread at a time. SaveThread handles the locking when
// transfering between threads. To enforce single ownership, they are passed 
// around via std::unique_ptr. Please respect this convention.
//
// By following this convention, if you hold the unique_ptr to a SaveBuffer,
// you can modify the struct without needing to do any additional locking.
struct SaveBuffer
{
	std::vector<unsigned char> data;
    
	int camera;
	SYSTEMTIME st;
	
    bool is_one_shot;
    int one_shot_tag;

    void store(void* src, size_t byte_count);
	void save();
	void clear();

  private:
    SaveBuffer() : camera(0), is_one_shot(false), one_shot_tag(0) {}
    friend class SaveThread;
};

class SaveThread
{
    std::thread m_thread;
    std::mutex  m_mutex;
    std::condition_variable m_cv;
    
    bool m_should_quit;
    
    std::string m_base_path;
    
    std::vector<std::unique_ptr<SaveBuffer> > save_queue;
    std::vector<std::unique_ptr<SaveBuffer> > free_buffers;
    
  public:
    SaveThread();
    
    // allocates buffer_count free buffers, each with iniital_data_reserve
    // bytes preallocated in their data fields. This is intended to be called
    // once when the thread is first started.
    void reserve_free_buffers(size_t buffer_count, size_t initial_data_reserve);
  
    // grabs a SaveBuffer from the free buffers list, or allocates a new one
    // if there are no free buffers available.
    std::unique_ptr<SaveBuffer> get_buffer();
    
    // takes ownership of the buffer and puts it on the save queue so that its
    // contents will be written to disk. ptr will be empty upon return.
    void save(std::unique_ptr<SaveBuffer>& ptr);
    
    // Equivalent to calling save() followed by get_buffer(), but avoids
    // the overhead of multiple mutex lock/unlocks. This is usually what you
    // want in most cases, except when first starting and when finishing.
    void save_and_get_buffer(std::unique_ptr<SaveBuffer>& ptr);
};




