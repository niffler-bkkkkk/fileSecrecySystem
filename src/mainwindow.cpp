#include "./headers/mainwindow.h"
#include "ui_mainwindow.h"
#include "./headers/rc4.h"
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QDebug>
#include <string>
#include <sstream>
#include <iostream>
#include "./src/playfair.cpp"
#include "./src/casear.cpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
//打开文件
void MainWindow::on_actionopen_triggered()
{
    QString fileName;
    fileName = QFileDialog::getOpenFileName(this,tr("open file"),"",tr("text(*.txt)"));
    if(!fileName.isNull()){
        QFile file(fileName);
        if(!file.open(QFile::ReadOnly|QFile::Text)){
            QMessageBox::warning(this,tr("Error"),tr("open file error").arg(file.errorString()));
            return;
        }
        QTextStream in(&file);
        QApplication::setOverrideCursor(Qt::WaitCursor);
        this->ui->inputEdit->setPlainText(in.readAll());
        QApplication::restoreOverrideCursor();
    }else{
        qDebug()<<"取消";
    }
}
//保存文件
void MainWindow::on_actionsave_as_triggered()
{
    QString fileName;

    fileName = QFileDialog::getSaveFileName(this,tr("save file"),".txt",tr("text(*.txt)"));
    if(!fileName.isNull()){
        QFile file(fileName);
        if(!file.open(QFile::ReadWrite|QFile::Text)){
            QMessageBox::warning(this,tr("Error"),tr("open file error").arg(file.errorString()));
            return;
        }
        QTextStream in(&file);
        in<<this->ui->outputEdit->toPlainText();
    }else{
        qDebug()<<"取消";
    }
}
//点击加密按钮
void MainWindow::on_encryptBtn_clicked(){
    time.start();//开始计时
    QString qstr=ui->inputEdit->toPlainText();
    QString pstr=ui->keyEdit->text();
    int status = statusJudge(qstr,pstr);//获取并判断当前状态
    if(status == 1){//缺少明文
        ui->statusBar->showMessage("Please input plain text !");
    }else if(status == 2){//缺少密钥
        ui->statusBar->showMessage("Please input key !");
    }else if(status == 0 | status == 3){
        string key=pstr.toStdString();
        char * oData;
        QByteArray da=qstr.toLatin1();
        oData=da.data();
        if(ui->encryptionMethod->currentText() == "RC4"){//根据用户选择调用不同的加密算法
           rc4 rc(qstr.toStdString(),pstr.toStdString(),"");
            QString input = textCleaner(qstr);
            string input_str = input.toStdString();
            QString input_qhex=QString::fromStdString(rc.string_to_hex(input_str));
            for(int i=0;i<input_qhex.length();i++){
                static const char* const lut="0123456789ABCDEF";
                string input_test=input_qhex.toStdString();
                char a = input_test[i];
                const char* p = std::lower_bound(lut, lut + 16, a);
            }
            rc.cipher(input_str,key,false);
            string output=rc.getEncoded();
            QByteArray data=QByteArray::fromStdString(output);
            ui->outputEdit->setPlainText(data.toHex());
            ui->statusBar->showMessage("Data encrypted!");
        }else if(ui->encryptionMethod->currentText() == "Blowfish"){
            blowfish.calcSubKey(pstr);
            QByteArray BfEncyptedData = blowfish.encrypt(QByteArray(qstr.toUtf8()));
            ui->outputEdit->clear();
            ui->outputEdit->setPlainText(BfEncyptedData.toBase64());
            ui->statusBar->showMessage("Data encrypted!");
        }else if(ui->encryptionMethod->currentText() == "XOR"){
            xorCipher.setKey(pstr);
            QByteArray XorEncyptedData = xorCipher.encrypt(QByteArray(qstr.toUtf8()));
            ui->outputEdit->clear();
            ui->outputEdit->setPlainText(XorEncyptedData);
            ui->statusBar->showMessage("Data encrypted!");
        }else if(ui->encryptionMethod->currentText() == "Playfair"){
            QStringList list=ui->inputEdit->toPlainText().split("\n");
            ui->outputEdit->clear();
            for(int i=0;i<list.size();i++){
            QString mes=list.at(i);
            string plainmes=mes.toStdString();
            string output=encode(plainmes,key);
            ui->outputEdit->appendPlainText(QString::fromStdString(output));
            }
            ui->statusBar->showMessage("Data encrypted!");
        }else if(ui->encryptionMethod->currentText() == "Casear"){
            int keylength=pstr.toInt();
            char* out=encrypt(oData,keylength);
            ui->outputEdit->setPlainText(out);
            ui->statusBar->showMessage("Data encrypted!");
        }else if(ui->encryptionMethod->currentText() == "AES"){
            if(status == 3){//密钥超过16字节
                ui->statusBar->showMessage("Secret key is not more than 16 bytes.");
            }else if(status == 0){
                aes.InputData(qstr.toStdString(),pstr.toStdString());
                ui->statusBar->clearMessage();
                string AESEncryptData = aes.Encrypt();
                ui->outputEdit->clear();
                ui->outputEdit->setPlainText(QString::fromUtf8(AESEncryptData.data(),AESEncryptData.size()));
                ui->statusBar->showMessage("Data encrypted!");
            }

        }
        speedTest();//测试运行时间
    }
}
//点击解密
void MainWindow::on_decryptBtn_clicked(){
    time.start();//开始计时
    QString qstr=ui->outputEdit->toPlainText();
    QString pstr=ui->keyEdit->text();
    int status = statusJudge(qstr,pstr);//获取并判断当前状态
    if(status == 1){//缺少密文
        ui->statusBar->showMessage("Please input cipher text !");
    }else if(status == 2){//缺少密钥
        ui->statusBar->showMessage("Please input key !");
    }else if(status == 0 | status == 3){
    qstr=qstr.simplified();
    string key=pstr.toStdString();
    char * oData;
    QByteArray da=qstr.toLatin1();
    oData=da.data();
    if(ui->encryptionMethod->currentText() == "RC4"){//根据用户选择调用不同的解密算法
        QByteArray da=qstr.toLatin1();
        QString in=QString::fromLatin1(da);
        QString input=textCleaner(in);
        rc4 rc("","","");
        string input_str=input.toStdString();
        input=QString::fromStdString(input_str);
        rc.decipher(input_str,key);
        string output=rc.getDecoded();
        QByteArray data=QByteArray::fromStdString(output);
        QString out=QString::fromLocal8Bit(data).toLower();
        ui->outputEdit->setPlainText(out);
    }else if(ui->encryptionMethod->currentText() == "RSA"){
        ui->outputEdit->clear();
        rsa_out="";
        for(int i=0;i<qstr.size();i=i+8){
           QString qs=qstr.mid(i,i+8);
           char* dat;
           QByteArray datt=qs.toLatin1();
           dat=datt.data();
           rsacipher.TestRSA(dat);
           string output=rsacipher.getcipher();
           rsa_out.append(rsacipher.getdecipher());
           ui->outputEdit->appendPlainText(QString::fromStdString(output));
        }
    }else if(ui->encryptionMethod->currentText() == "Blowfish"){
        blowfish.calcSubKey(pstr);
        QByteArray BfDecyptedData = blowfish.decrypt(QByteArray::fromBase64(QByteArray(qstr.toUtf8())));
        ui->outputEdit->clear();
        ui->outputEdit->setPlainText(BfDecyptedData);
    }else if(ui->encryptionMethod->currentText() == "XOR"){
        xorCipher.setKey(pstr);
        QString XorInputData = textCleaner(qstr);
        QByteArray XorDecyptedData = xorCipher.decrypt(QByteArray(XorInputData.toUtf8()));
        ui->outputEdit->clear();
        ui->outputEdit->setPlainText(XorDecyptedData);
    }else if(ui->encryptionMethod->currentText() == "Casear"){
        int keylength=pstr.toInt();
        QStringList list=ui->outputEdit->toPlainText().split("\n");
        ui->outputEdit->clear();
        for(int i=0;i<list.size();i++){
            QString mes=list.at(i);
            char* data;
            QByteArray dat=mes.toLatin1();
            data=dat.data();
            char* out=encrypt(data,-keylength);
            ui->outputEdit->appendPlainText(out);
        }
    }else if(ui->encryptionMethod->currentText() == "Playfair"){
        QStringList list=ui->outputEdit->toPlainText().split("\n");
        ui->outputEdit->clear();
        for(int i=0;i<list.size();i++){
            QString mes=list.at(i);
            string plainmes=mes.toStdString();
            string output=decode(plainmes,key);
            ui->outputEdit->appendPlainText(QString::fromStdString(output));
        }
    }else if(ui->encryptionMethod->currentText() == "AES"){
        if (status == 3) {//密钥超过16字节
            ui->statusBar->showMessage("Secret key is not more than 16 bytes.");
        }else if(status == 0) {
            aes.InputData(qstr.toStdString(),pstr.toStdString());
            ui->statusBar->clearMessage();
            string AESDecrypt = aes.Decrypt();
            ui->outputEdit->clear();
            ui->outputEdit->setPlainText(QString::fromUtf8(AESDecrypt.data(),AESDecrypt.size()));
        }
    }else if(ui->encryptionMethod->currentText() == "RSA"){
        ui->outputEdit->clear();
        QByteArray dae=QByteArray::fromStdString(rsa_out);
        char* out=dae.data();
        ui->outputEdit->setPlainText(dae);
    }
    ui->statusBar->showMessage("Data decrypted!");
    speedTest();//测试运行时间
  }
}
