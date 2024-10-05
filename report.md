## Oct 7. Progress and Problems
This week, I optimized the `merge` routine, so that it no longer generates an exponential number of sub-tasks.

The original implementation has a problem of generating too much subtasks **and then depending on them**. The result is that all working threads are waiting for their children to complete. However, the newly-submitted child tasks does not have a thread to use. This would cause a deadlock.

One way to remedy this is to use a more sophisticated thread pool implementation, rather than using a simple spinlock. This would, in theory, permit sleeping threads to avoid consuming system resources. However, I wanted to find a way that does not involve rewriting the whole task dispatcher.