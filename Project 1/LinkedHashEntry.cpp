class LinkedHashEntry {
private:
      int key;
      int value;
      LinkedHashEntry *next;
public:
      LinkedHashEntry(int key, int value) {
            this->key = key;
            this->value = value;
            this->next = NULL;
      }
 
      int getKey() {
            return key;
      }
 
      int getValue() {
            return value;
      }
 
      void setValue(int value) {
            this->value = value;
      }
 
      LinkedHashEntry *getNext() {
            return next;
      }
 
      void setNext(LinkedHashEntry *next) {
            this->next = next;
      }
};