## WorkThread ##

> WorkThread是一个工作者线程，从Thread继承。当线程启动后不断从任务队列中获取任务，然后进行处理。当任务队列空时，线程等待条件变量而进入休眠状态，直到收到添加任务的条件信号。WorkThread特别适用于生产者/消费者模式。
```
template <class T>
class WorkThread: public Thread<T>
{
protected:
	////实现Thread接口:发送添加任务事件
	bool notify_add_task();
	////实现Thread接口:响应添加任务事件
	bool on_notify_add_task();
	////实现Thread接口:线程实际运行的入口
	void run_thread();
protected:
	////线程处理任务接口
	void handle_task(T &task)=0;
public:
	WorkThread();
	~WorkThread();
private:
	pthread_cond_t m_cond;
	pthread_mutex_t m_cond_mutex;
};

////////////////////////////////////////////////
template <class T>
WorkThread<T>::WorkThread()
{
	pthread_cond_init(&m_cond, NULL);
	pthread_mutex_init(&m_cond_mutex, NULL);
}

template <class T>
WorkThread<T>::~WorkThread()
{
	pthread_cond_destroy(&m_cond);
	pthread_mutex_destroy(&m_cond_mutex);
}

template <class T>
bool WorkThread<T>::notify_add_task()
{
	pthread_mutex_lock(&m_cond_mutex);
	pthread_cond_signal(&m_cond);
	pthread_mutex_unlock(&m_cond_mutex);
	return true;
}

template <class T>
bool WorkThread<T>::on_notify_add_task()
{
	//等待任务队列非空
	pthread_mutex_lock(&m_cond_mutex);
	pthread_cond_wait(&m_cond, &m_cond_mutex);
	pthread_mutex_unlock(&m_cond_mutex);
	return true;
}

template <class T>
void WorkThread<T>::run_thread()
{
	this->set_thread_ready();

	while(true)
	{
		on_notify_add_task();
		T task;
		while(get_task(task))
			handle_task(task);
	}
}

```