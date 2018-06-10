#include "examples/rapid_reassignment/sender_rr.hpp"

namespace flexps {
Sender_rr::Sender_rr(AbstractMailbox* mailbox) : mailbox_(mailbox) {}

void Sender_rr::Start() {
  sender_thread_ = std::thread([this] { Send(); });
}

void Sender_rr::Send() {
  while (true) {
    Message to_send;
    send_message_queue_.WaitAndPop(&to_send);
    if (to_send.meta.flag == Flag::kExit)
      break;
    mailbox_->Send(to_send);
  }
}

ThreadsafeQueue<Message>* Sender_rr::GetMessageQueue() { return &send_message_queue_; }

void Sender_rr::Stop() {
  Message stop_msg;
  stop_msg.meta.flag = Flag::kExit;
  send_message_queue_.Push(stop_msg);
  sender_thread_.join();
}

}  // namespace flexps
