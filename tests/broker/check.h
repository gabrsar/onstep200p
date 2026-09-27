int main() {
  assert(commandBroker.init());
  assert(commandBroker.send(":RS#"));
  assert(commandBroker.send(":Mw#"));
  commandBroker.poll(); // Frees slot 0, while movement remains in slot 1.
  assert(commandBroker.send(":Q#")); // Reuses slot 0; must NOT overtake Mw.
  commandBroker.poll();
  commandBroker.poll();
  assert((serialStub.sent==std::vector<std::string>{":RS#",":Mw#",":Q#"}));
  for (int i=0;i<COMMAND_BROKER_SLOTS;++i) assert(commandBroker.send(":Q#"));
  assert(!commandBroker.send(":Q#"));
  for (int i=0;i<COMMAND_BROKER_SLOTS;++i) commandBroker.poll();
  assert(commandBroker.send(":Q#"));
}
