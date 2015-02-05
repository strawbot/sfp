bool spiRx(void);
Byte spiGet(void);
bool spiTx(void);
void spiPut(Long b);

void initSpiRxq(void);
void serviceSpiTxLink(linkInfo_t *spilink);
void serviceSpiRx(void);
