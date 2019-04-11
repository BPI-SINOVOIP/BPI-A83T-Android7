#pragma once

#include <pthread.h>

namespace allwinner
{

bool MatchesHead( const char * head, const char * check );

class MessageQueue
{
public:
                    MessageQueue( int maxMessages );
                    ~MessageQueue();

    // Shut down the message queue once messages are no longer polled
    // to avoid overflowing the queue on message spam.
    void            Shutdown();

    // Thread safe, callable by any thread.
    // The msg text is copied off before return, the caller can free
    // the buffer.
    // The app will abort() with a dump of all messages if the message
    // buffer overflows.
    void            PostString( const char * msg );

    // Builds a printf string and sends it as a message.
    void            PostPrintf( const char * fmt, ... );

    // The other methods are NOT thread safe, and should only be
    // called by the thread that owns the MessageQueue.

    // Returns NULL if there are no more messages, otherwise returns
    // a string that the caller is now responsible for freeing.
    const char *     GetNextMessage();

    // Returns immediately if there is already a message in the queue.
    void            SleepUntilMessage();

    // Dumps all unread messages
    void            ClearMessages();

public:
    // If set true, print all message sends and gets to the log
    static bool        debug;

    bool            shutdown;
    const int         maxMessages;

    // All messages will be allocated with strdup, and returned to
    // the caller on GetNextMessage().
    const char **     messages;

    // PostMessage() fills in messages[tail%maxMessages], then increments tail
    // If tail > head, GetNextMessage() will fetch messages[head%maxMessages],
    // then increment head.
    int                head;
    int                tail;

    pthread_mutex_t mutex;
    pthread_cond_t    wake;
};

}    // namespace allwinner
