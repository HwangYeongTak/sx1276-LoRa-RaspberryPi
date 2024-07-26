#include "LoRa.h"

struct timespec last_time;
struct timespec current_time;

void tx_f(txData *tx){

    clock_gettime(CLOCK_MONOTONIC, &current_time); // 현재 시간 기록
    // printf("\n# Send end time: %ld.%09ld seconds\n", current_time.tv_sec, current_time.tv_nsec);
    // 송신 시간 출력
    if (last_time.tv_sec != 0 || last_time.tv_nsec != 0) {
        long seconds = current_time.tv_sec - last_time.tv_sec;
        long nanoseconds = current_time.tv_nsec - last_time.tv_nsec;
        double elapsed = seconds + nanoseconds * 1e-9;
        printf("# Sending Time : %.9f seconds\n", elapsed);
    }
    LoRa_ctl *modem = (LoRa_ctl *)(tx->userPtr);
    // printf("tx done;\t");
    // printf("sent string: \"%s\"\n", tx->buf);//Data we've sent

    clock_gettime(CLOCK_MONOTONIC, &current_time); // 현재 시간 기록
    printf("\n# Receive start time: %ld.%09ld seconds\n", current_time.tv_sec, current_time.tv_nsec);

    LoRa_receive(modem);
}

void * rx_f(void *p){

    clock_gettime(CLOCK_MONOTONIC, &current_time); // 현재 시간 기록
    printf("\n# Receive end time: %ld.%09ld seconds\n\n", current_time.tv_sec, current_time.tv_nsec);

    rxData *rx = (rxData *)p;
    LoRa_ctl *modem = (LoRa_ctl *)(rx->userPtr);

    LoRa_stop_receive(modem);//manually stoping RxCont mode

    // printf("rx done;\t");
    // printf("CRC error: %d;\t", rx->CRC);
    // printf("Data size: %d;\t", rx->size);
    // printf("received string: \"%s\";\t", rx->buf);//Data we've received
    printf("RSSI: %d;\t", rx->RSSI);
    // printf("SNR: %f\n\n", rx->SNR);
    
    // memcpy(modem->tx.data.buf, "123", 4);//copy data we'll sent to buffer
    // modem->tx.data.size = 4;//Payload len

    sleep(1); // 전송 시작에 딜레이를 줌

    clock_gettime(CLOCK_MONOTONIC, &current_time); // 현재 시간 기록
    printf("\n# Send start time: %ld.%09ld seconds\n", current_time.tv_sec, current_time.tv_nsec);
    last_time = current_time; 
    LoRa_send(modem);
    
    // printf("Time on air data - Tsym: %f;\t", modem->tx.data.Tsym);
    // printf("Tpkt: %f;\t", modem->tx.data.Tpkt);
    // printf("payloadSymbNb: %u\n", modem->tx.data.payloadSymbNb);
    printf("sleep %.2f seconds to transmit complete\n", modem->tx.data.Tpkt / 1000.0);
    free(p);

    return NULL;
}

int main(){

    char txbuf[255];
    char rxbuf[255];
    LoRa_ctl modem;

    int header_mode;
    printf("Choose Header Mode\n");
    printf("0: Explicit header mode | 1: Implicit header mode = ");
    scanf("%d", &header_mode);
    if (header_mode != 0 && header_mode != 1) { return 1; }

    //See for typedefs, enumerations and there values in LoRa.h header file
    modem.spiCS = 0;//Raspberry SPI CE pin number
    modem.tx.callback = tx_f;
    modem.tx.data.buf = txbuf;
    modem.rx.callback = rx_f;
    modem.rx.data.userPtr = (void *)(&modem);//To handle with chip from rx callback
    modem.tx.data.userPtr = (void *)(&modem);//To handle with chip from tx callback

    memcpy(modem.tx.data.buf, "123", 4);//copy data we'll sent to buffer
    if (header_mode == 1) {
        modem.eth.payloadLen = 4; //payload len used in implicit header mode
    }
    else { 
        modem.tx.data.size = 4;//Payload len
    }

    modem.eth.preambleLen=6;
    modem.eth.bw = BW125;//Bandwidth 62.5KHz
    modem.eth.sf = SF12;//Spreading Factor 12
    modem.eth.ecr = CR8;//Error coding rate CR4/8
    modem.eth.CRC = 1;//Turn on CRC checking
    modem.eth.freq = 915000000;// 434.8MHz
    modem.eth.resetGpioN = 4;//GPIO4 on lora RESET pin
    modem.eth.dio0GpioN = 17;//GPIO17 on lora DIO0 pin to control Rxdone and Txdone interrupts
    modem.eth.outPower = OP20;//Output power
    modem.eth.powerOutPin = PA_BOOST;//Power Amplifire pin
    modem.eth.AGC = 1;//Auto Gain Control
    modem.eth.OCP = 240;// 45 to 240 mA. 0 to turn off protection
    modem.eth.implicitHeader = header_mode;//Explicit header mode
    modem.eth.syncWord = 0x12;
    //For detail information about SF, Error Coding Rate, Explicit header, Bandwidth, AGC, Over current protection and other features refer to sx127x datasheet https://www.semtech.com/uploads/documents/DS_SX1276-7-8-9_W_APP_V5.pdf

    clock_gettime(CLOCK_MONOTONIC, &current_time); // 현재 시간 기록
    printf("\n# LoRa_begin time: %ld.%09ld seconds\n", current_time.tv_sec, current_time.tv_nsec);
    LoRa_begin(&modem);
    LoRa_receive(&modem);


    while(1) { sleep(1); }
    printf("end\n");



    LoRa_end(&modem);
}

