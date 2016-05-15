Emotiv EEGlogger 框架, 读取以及保存各个通道实时的读值
注：建议选用HyperNeuro的HiBrain代替之（从性能和便携性方向考虑，毕竟Emotiv的硬件和软件已经是老一代了）

/// 记录下各个通道的信息，以及使用Expressiv TM实时读出面部表情
//运行:  >> EEGlogger.exe dh // dh为存储ofs的地方ie ofs
#include <iostream>
#include <fstream>
#include <conio.h>
#include <sstream>
#include <windows.h>
#include <map>
#include <stdio.h> //这里只是为了调用清屏

#include "EmoStateDLL.h"
#include "edk.h"
#include "edkErrorCode.h"

#pragma comment(lib, "../lib/edk.lib")

EE_DataChannel_t targetChannelList[] = {  //通道列表
        ED_COUNTER,    //0
        ED_AF3, ED_F7, ED_F3, ED_FC5, ED_T7, //1--5
        ED_P7, ED_O1, ED_O2, ED_P8, ED_T8,   //6--10
        ED_FC6, ED_F4, ED_F8, ED_AF4, ED_GYROX, ED_GYROY, ED_TIMESTAMP, //11-17
        ED_FUNC_ID, ED_FUNC_VALUE, ED_MARKER, ED_SYNC_SIGNAL    //18--21
    };

const char header[] = "COUNTER,AF3,F7,F3, FC5, T7, P7, O1, O2,P8"        //写入ofs 数据库格式
                      ", T8, FC6, F4,F8, AF4,GYROX, GYROY, TIMESTAMP, "
                      "FUNC_ID, FUNC_VALUE, MARKER, SYNC_SIGNAL,";
double Counter[128],AF3[128],F7[128],F3[128],FC5[128],T7[128],P7[128],O1[128],
    O2[128],P8[128],T8[128],FC6[128],F4[128],F8[128],AF4[128]; // 另外2个是reference，一共有用的是14个channels
// 上面那14个通道,1个counter, 保存了最新1s内128个采样的读值, 而下面ofs则保存了历史以来的值.
//
int main(int argc, char** argv) {

    EmoEngineEventHandle eEvent            = EE_EmoEngineEventCreate();    // 可用来看是否有更新
    EmoStateHandle eState                = EE_EmoStateCreate();            // 可用来读取Expressive
    unsigned int userID                    = 0;        // 用户输入的用户名
    const unsigned short composerPort    = 1726;        //用Composer时的端口
    float secs                            = 1;        //Bufer Size in Second    每秒采样率    获取原始采样数据用 设备每秒每通道采样128次，若1s，则每个通道每次更新128个采样
    unsigned int datarate                = 0;        //没有用到过                获取原始采样数据用
    bool readytocollect                    = false;    //Enable 采样标识为            获取原始采样数据用
    int option                            = 0;        //用户输入的选择，1：EmoEngine 2：EmoComposer
    int state                            = 0;        //while loop中用以检查即时事件


    std::string input;

    try {

        if (argc != 2)  // 若启动时没有给出UserID则出错,立即退出
        {    throw std::exception("Please supply the log file name.\nUsage: EEGLogger.exe [log_file_name].");}

        std::cout << "===================================================================" << std::endl;
        std::cout << "Example to show how to log EEG Data from EmoEngine/EmoComposer."       << std::endl;
        std::cout << "===================================================================" << std::endl;
        std::cout << "Press '1' to start and connect to the EmoEngine                    " << std::endl;
        std::cout << "Press '2' to connect to the EmoComposer                            " << std::endl;
        std::cout << ">> ";

        std::getline(std::cin, input, '\n');
        option = atoi(input.c_str());    // 输入载入方式选择
        /*--------------------------------------------------------------------------------------------------------*/
        switch (option) {
            case 1:    // 若用户选择用 EmoEngine，但连接不了，则出错退出
            {
                if (EE_EngineConnect() != EDK_OK)
                {    throw std::exception("Emotiv Engine start up failed.");}
                break;
            }
            case 2: // 若用户选择用EmoComposer远程连接，但连接不了，则出错退出
            {
                std::cout << "Target IP of EmoComposer? [127.0.0.1] ";    // 默认连接IP
                std::getline(std::cin, input, '\n');
                if (input.empty())
                {    input = std::string("127.0.0.1");    }// 默认连接IP
                if (EE_EngineRemoteConnect(input.c_str(), composerPort) != EDK_OK)     // 若该IP连接不了，则出错退出
                {    std::string errMsg = "Cannot connect to EmoComposer on [" + input + "]";
                    throw std::exception(errMsg.c_str());}
                break;
            }
            default:    // 若用户输入选择不存在，则出错退出
                throw std::exception("Invalid option...");
                break;
        }// switch(option)
        /*--------------------------------------------------------------------------------------------------------*/
        /// 成功连接，下面开始正常运行
        system( "cls"); // 清空屏幕
        std::cout << "Start receiving EEG Data! Press any key to stop logging...\n" << std::endl;
        std::ofstream ofs(argv[1],std::ios::trunc);
        ofs << header << std::endl;

        DataHandle hData = EE_DataCreate();    // 建立 Data Handle 用以接入    采样用
        EE_DataSetBufferSizeInSec(secs);    // 每秒钟获取采样次数            采样用

        std::cout << "Buffer size in secs:" << secs << std::endl;
        std::cout << "Starting..." << std::endl;
        Sleep(1000);
        system( "cls"); // 清空屏幕
        /*--------------------------------------------------------------------------------------------------------*/
        while (!_kbhit())     //////// 当任意敲击键盘，则退出死循环
        {
            state = EE_EngineGetNextEvent(eEvent);    // 查看Engine状态
            if (state == EDK_OK)     // 更新最新事件，以及判断是否开始采样收集
            {    EE_Event_t eventType = EE_EmoEngineEventGetType(eEvent); // 查看Engine事件
                EE_EmoEngineEventGetUserId(eEvent, &userID);
                // Log the EmoState if it has been updated 若新接入则启动Enable
                if (eventType == EE_UserAdded)
                {    std::cout << "User added";
                    EE_DataAcquisitionEnable(userID,true);    // Enable 采样
                    readytocollect = true;    // Enable 采样
                    }
            }

            if (readytocollect)     // 收集最新采样数据，并记录下来
            {            EE_DataUpdateHandle(0, hData);    // 更新EEG信号到hData中
                        unsigned int nSamplesTaken=0;
                        EE_DataGetNumberOfSample(hData,&nSamplesTaken); // 查看执行了多少次采样, 默认1s是128次，其实根本不用查询的
                        //std::cout << "----Update-----" << std::endl;
                        //std::cout << "Updated " << nSamplesTaken <<" Samples." << std::endl;// 显示更新的Samples数量( 1s 默认是128次)

                        if (nSamplesTaken != 0)     //记录下新获取的Samples
                        {    double* data = new double[nSamplesTaken];    // Buffer
                            for (int sampleIdx=0 ; sampleIdx<(int)nSamplesTaken ; ++ sampleIdx)        // sampleIndex 0--128
                            {    for (int i = 0 ; i<sizeof(targetChannelList)/sizeof(EE_DataChannel_t) ; i++) // i Channel Index
                                {    EE_DataGet(hData, targetChannelList[i], data, nSamplesTaken);// Extract data from the hData

                                    //AF3,F7,F3,FC5,T7,P7,O1,  O2,P8,T8,FC6,F4,F8,AF4///////提取最新1s的采样////////
                                    switch (i) {    //注：data被=一次后，里面变成0，不过不会影响ofs的保存
                                            case 0:{Counter[sampleIdx]=data[sampleIdx];    break;}
                                            case 1:{AF3[sampleIdx]=data[sampleIdx]; break;}
                                            case 2:{F7[sampleIdx]=data[sampleIdx]; break;}
                                            case 3:{F3[sampleIdx]=data[sampleIdx]; break;}
                                            case 4:{FC5[sampleIdx]=data[sampleIdx]; break;}
                                            case 5:{T7[sampleIdx]=data[sampleIdx]; break;}
                                            case 6:{P7[sampleIdx]=data[sampleIdx]; break;}
                                            case 7:{O1[sampleIdx]=data[sampleIdx]; break;}
                                            case 8:{O2[sampleIdx]=data[sampleIdx]; break;}
                                            case 9:{P8[sampleIdx]=data[sampleIdx]; break;}
                                            case 10:{T8[sampleIdx]=data[sampleIdx]; break;}
                                            case 11:{FC6[sampleIdx]=data[sampleIdx]; break;}
                                            case 12:{F4[sampleIdx]=data[sampleIdx]; break;}
                                            case 13:{F8[sampleIdx]=data[sampleIdx]; break;}
                                            case 14:{AF4[sampleIdx]=data[sampleIdx]; break;}
                                            default: break;
                                    ofs << data[sampleIdx] << ","; //存储进ofs文件，则运行时若是 XXX.exe dh ，则存在dh文件中
                                }
                                ofs << std::endl;    // 每次采样空一行，则128次每秒的采样，1s有128行,Counter就是采样index
                            }
                            delete[] data;
                        }
            }

            Sleep(1000/secs); // delay
            system( "cls"); // 清空屏幕
            ///////////////////////////////////////////////////////////////////////////

            //显示最新128个AF3采样，测试用, 可以和保存的ofs对比一下, 发现是一样的
            std::cout << "-----Channels-----" << std::endl;
            std::cout<<"Counter  "<<"AF3"<<std::endl;
            for(int i=0;i<128;i++)
            {    std::cout<<Counter[i]<<' '<<AF3[i]<<std::endl; }
            ///////////////////////////////////////////////////////////////////////////
             std::cout << "-----Expressiv-----" << std::endl;
             EE_EngineGetNextEvent(eEvent);      // 更新 事件 （可以去掉）
             EE_EmoEngineEventGetEmoState(eEvent, eState);     // 更新 状态
             if (ES_ExpressivIsBlink(eState)) {std::cout << "Blink" << std::endl;}    // 读出面部状态
             if (ES_ExpressivIsLeftWink(eState)) {std::cout << "Left Wink" << std::endl;}
             if (ES_ExpressivIsRightWink(eState)) {std::cout << "Right Wink" << std::endl;}
             if (ES_ExpressivIsLookingRight(eState)) {std::cout << "Looking Right" << std::endl;}
             if (ES_ExpressivIsLookingLeft(eState)) {std::cout << "Looking Left" << std::endl;}

        } // while(!_kbhit()) 任意敲击键盘退出
        /*--------------------------------------------------------------------------------------------------------*/
        ofs.close();    // 退出时，关闭
        EE_DataFree(hData);
    }
    catch (const std::exception& e)     // 当载入出错，跳到这里退出
    {    std::cerr << e.what() << std::endl;
        std::cout << "Press any key to exit..." << std::endl;
        getchar();
    }

    EE_EngineDisconnect();    // 退出程序
    EE_EmoStateFree(eState);
    EE_EmoEngineEventFree(eEvent);

    return 0;
}
