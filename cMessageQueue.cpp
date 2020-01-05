
#include "cMessageQueue.h"

mqd_t cMessageQueue::m_mqWorker = -1;


mqd_t* cMessageQueue::vCreateMessageQueue()
{
    struct mq_attr attr;
      int    flags = O_RDWR | O_CREAT;
  mode_t mode  = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

    /* initialize the queue attributes */
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MSG_SIZE;
    attr.mq_msgsize = MAX_QUEUE_SIZE;
    attr.mq_curmsgs = 0;

    /* create the message queue */
    m_mqWorker = mq_open(QUEUE_NAME, flags , mode, &attr);
    CHECK((mqd_t)-1 != m_mqWorker);
    return &m_mqWorker;

}


int cMessageQueue::iReceiveMessage(char* buffer)
{

	int bytes_read = 0;
	 memset(buffer, 0, MAX_MSG_SIZE);
	/* receive the message */
	if(m_mqWorker != -1)
	bytes_read = mq_receive(m_mqWorker, buffer, MAX_QUEUE_SIZE, NULL);

	return bytes_read;
}

void cMessageQueue::vPostMessage(char* data)
{
	/* send the message */
	if(m_mqWorker != -1)
    {

    }
	CHECK(0 <= mq_send(m_mqWorker, data, MAX_MSG_SIZE, 0));

    /* cleanup */
    //CHECK((mqd_t)-1 != mq_close(m_mqWorker));

}


void cMessageQueue::vCloseMessage(char* data)
{
    /* cleanup */
    CHECK((mqd_t)-1 != mq_close(m_mqWorker));


}
