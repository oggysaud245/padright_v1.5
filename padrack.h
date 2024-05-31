#ifndef padrack_h
#define padrack_h
#endif
class padrack{
    int quantity = 0;
    int maxQuantity;
    public:
    int getQuantity();
    int getMaxQuantity();
    void setQuantity(int number);
    void setMaxQuantity(int maxQuantity);
    void decQuantity();
    void incQuantity();
    void setZero();

};
