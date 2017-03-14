#include "qt_json_socket_lib.h"


Qt_Json_Socket_Lib::Qt_Json_Socket_Lib():send_timeout__(-1)
  ,recv_timeout__(-1)
  ,sock__(nullptr)
{

}

Qt_Json_Socket_Lib::~Qt_Json_Socket_Lib()
{
    disconnect (sock__,SIGNAL(disconnected()), this, SLOT(disconnect_slot()));
    delete sock__;
}

bool Qt_Json_Socket_Lib::make_QTcpSocket()
{

    sock__ = new QTcpSocket;

    //disconnect 시그널 연결
    connect(sock__,SIGNAL(disconnected()), this, SLOT(disconnect_slot()));
}

void Qt_Json_Socket_Lib::set_socket(QTcpSocket *sock)
{

    //만약 sock__에 메모리가 할당이 되어있을경우
    if(sock__ != nullptr){
        qDebug()<<"[Error]: sock__ is not null_ptr";

        //해당 소켓을 delete함
        sock__->deleteLater ();
        //return;
    }

    sock__ = sock;

    //disconnect 시그널을 연결
    connect (sock__,SIGNAL(disconnected()), this, SLOT(disconnect_slot()));
}

QTcpSocket *Qt_Json_Socket_Lib::get_socket()
{
    return sock__;
}

bool Qt_Json_Socket_Lib::send_QByteArray(QByteArray arr)
{
    try{
        //소켓이 연결이 되어있는지 확인
        if(sock__->state () == QAbstractSocket::UnconnectedState){
            qDebug()<<"[Info]: socket is not connect";
            return false;
        }

        //소켓 바이트 사이즈를 QString으로 변환
        QString byte_Size = QString::number (arr.size ());
        qDebug()<<"[Info] :Byte size :"<<arr.size();
        qDebug()<<"[Info] :Byte size to String : "<<byte_Size;


        //QString을 utf8로 변환하여 전송
        sock__->write (byte_Size.toUtf8 (), sizeof(uint));
        qDebug()<<"[Info] : send buffer byte size";


        //QByteArray를 전송
        sock__->write (arr);
        if(sock__->waitForBytesWritten (send_timeout__) != true){
            qDebug()<<"[Info] : write Timeout";
            return false;
        }
        qDebug()<<"[Info] : send buffer";


        qDebug()<<"[Info]: Complete send QByteArray";
        return true;


    }catch(QAbstractSocket::SocketError e){
        qDebug()<<"[Error] : send_QByteArray Error : "<<sock__->errorString ();
        return false;
    }
}

QByteArray Qt_Json_Socket_Lib::recv_QByteArray()
{
    try{

        //소켓이 연결이 되어있는지 확인
        if(sock__->state () == QAbstractSocket::UnconnectedState){
            qDebug()<<"[Info]: socket is not connect";
            return "NULL";
        }


        //버퍼 사이즈를 읽음
        //만약 소켓이 읽는 타임아웃이 될경우
        if(sock__->waitForReadyRead (recv_timeout__) != true){
            qDebug()<<"[Error] : Time_out ";
            throw sock__->error ();
        }
        qDebug()<<"[Info] : Reading recv_siz";

        QByteArray recv_siz = sock__->read (sizeof(uint));
        qDebug()<<"[Info] : recv_byte_siz : "<<recv_siz.toUInt ();



        //버퍼 읽음
        qDebug()<<"[Info] : Reading recv_buffer";

        QByteArray recv_byte = sock__->read (recv_siz.toUInt ());
        qDebug()<<"[Info] : recv_buffer"<<recv_byte;

        //성공적으로 읽음 바이트를 리턴함
        qDebug()<<"[Info] : Complete recv buffer";
        return recv_byte;


    }catch(QAbstractSocket::SocketError e){
        qDebug()<<"[Error] : recv_QByteArray Error : "<<sock__->errorString ();
        return "NULL";
    }
}

bool Qt_Json_Socket_Lib::send_Json(QJsonObject obj)
{

    //QJsonObject를 QJsonDocument로 변환
    doc__.setObject (obj);

    if(this->send_QByteArray (doc__.toBinaryData ()) != true){
        qDebug()<<"[Info] : Error of sending QDocument ";
        return false;
    }

    qDebug()<<"[Info] : send_Json Success";
    return true;
}

QJsonDocument Qt_Json_Socket_Lib::recv_Json()
{
    QByteArray arr = this->recv_QByteArray ();

    QJsonDocument Json_doc;

    //만약 arr의 리턴값이 NULL일 경우
    if(arr == "NULL"){
        qDebug()<<"[Info] : Error of recv_Json";

        //json_doc를 empty로 반환
        return Json_doc;
    }

    //수신받은 QByteArray를 재조합
    Json_doc = QJsonDocument::fromBinaryData (arr);

    //만약 QByteArray의 재조합에 실패했을 경우
    if(Json_doc.isNull ()){
        qDebug()<<"[Error]: QJsonObject structure is wrong ";

        //json_doc를 empty로 반환
        return Json_doc;
    }

    //정상적으로 변환 완료됬음
    qDebug()<<"[Info] : success recv Json_object";

    return Json_doc;



}

void Qt_Json_Socket_Lib::set_connect_timeout(qint64 t)
{
    connect_timeout__ = t;
}

void Qt_Json_Socket_Lib::set_send_timeout(qint64 t)
{
    send_timeout__ = t;
}

void Qt_Json_Socket_Lib::set_recv_timeout(qint64 t)
{
    recv_timeout__ = t;
}

void Qt_Json_Socket_Lib::connect_server(QHostAddress &a, qint64 port)
{
    sock__->connectToHost (a, port);

    if(sock__->waitForConnected (connect_timeout__) != true){
        qDebug()<<"[Info] : didn't connect to server : "<<sock__->errorString ();
        return;
     }

    qDebug()<<"[Info] : success connect to server";


}

//정상적으로 disconnect를 했을경우
void Qt_Json_Socket_Lib::disconnect_server()
{

    qDebug()<<"[Info] : disconnecting_server";

    sock__->close ();
}

void Qt_Json_Socket_Lib::disconnect_socket()
{
    qDebug()<<"[Info] : disconnect_socket";

    sock__->close ();
}

void Qt_Json_Socket_Lib::delete_QTcpSocket()
{
    sock__->deleteLater ();

}


//### slot area ####
//만약 서버로부터 disconnect의 시그널이 왔을경우
void Qt_Json_Socket_Lib::disconnect_slot()
{
    //disconnect_from_socket의 시그널을 발생
    emit disconnect_from_socket ();
}
