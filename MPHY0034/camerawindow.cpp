#include "camerawindow.h"
#include "ui_camerawindow.h"
#include <QWidget>
#include <QString>

//窗口构造函数
CameraWindow::CameraWindow(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::CameraWindow)
    //自定义参数的初始化
    , ObjDevicePtr(NULL)
    , ObjStreamPtr(NULL)
    , ObjFeatureControlPtr(NULL)
    , pCaptureEventHandler(NULL)
    , m_pBitmap(NULL)
    , m_bCheckSaveBmp(false)
    , ImgWidth(0)
    , ImgHeight(0)
    , TimePushButton(0)
    , TimeTakePhoto(0)
{
    ui->setupUi(this);
    LabelShowCurImg = ui->label_CurInput;
    LabelSingleImg = ui->label_SampleImg;//这两个指针的赋值务必放在setupUi的后面，否则前面没有对应的指针，就是空的
    LabelSingleImg->setScaledContents(true);//设置比例

    //实时
    pDrawWidget = ui->widget_CurInput;
    pDrawWidget->SetSize();
    //LabelShowCurImg->hide();

    ui->lineEdit_ExplosureTime->setText("50000");
    ui->lineEdit_FpsSet->setText("8");
    ui->lineEdit_FilePath->setText(".\\data");
    ui->lineEdit_CamIndex->setText("1");//序号
    ui->lineEdit_FileName->setText("test.bmp");

    this->CamWinParamsSet(800, 800, 100, 30);//窗口
    this->GeoSetWidgetCurInput(100, 100, 400, 400);//实时参数
    this->GeoSetWidgetSampleImg(600, 600,400, 400);//采集参数
    this->GeoSetFrameParamsSet(100, 910, 500, 100, 10, 10);//参数框参数
}

CameraWindow::~CameraWindow()
{
    delete ui;
}


/* **************************************图形界面几何设计***********************************************/

//宽高
void CameraWindow::CamWinParamsSet(int _WidgetW, int _WidgetH, int _SmallPartW, int _SmallPartH)
{
     
    CwParas.WidgetWidth = _WidgetW;
    CwParas.WidgetHeight = _WidgetH;
    CwParas.LabelWidth = _SmallPartW;
    CwParas.LabelHeight = _SmallPartH;
    CwParas.LineeditWidth = _SmallPartW;
    CwParas.LineeditHeight = _SmallPartH;
    CwParas.ButtonWidth = _SmallPartW;
    CwParas.ButtonHeight = _SmallPartH;
}


//实时widget
void CameraWindow::GeoSetWidgetCurInput(int _x, int _y, int _width, int _height)
{


 
    ui->widget_CurInput->move(_x, _y);
    ui->widget_CurInput->resize(_width, _height);

 
    ui->label_CurInput->move(0, 0);
    ui->label_CurInput->resize(_width, _height);
}

void CameraWindow::GeoSetWidgetSampleImg(int _x, int _y, int _width, int _height)
{



    ui->widget_SampleImg->move(_x, _y);
    ui->widget_SampleImg->resize(_width, _height);

    //设置label
    ui->label_SampleImg->move(0, 0);
    ui->label_SampleImg->resize(_width, _height);
}

//参数框
void CameraWindow::GeoSetFrameParamsSet(int _x, int _y, int _width, int _height, int _dx, int _dy)
{

    ui->frame_ParmsSet->move(_x, _y);
    ui->frame_ParmsSet->resize(_width, _height);

    ui->label_ExplosureTime->move(_dx, _dy);
    ui->label_ExplosureTime->resize(CwParas.LabelWidth, CwParas.LabelHeight);
    ui->label_FpsSet->move(_dx, 2 * _dy + CwParas.LabelHeight);
    ui->label_FpsSet->resize(CwParas.LabelWidth, CwParas.LabelHeight);

    ui->lineEdit_ExplosureTime->move(2 * _dx + CwParas.LabelWidth, _dy);
    ui->lineEdit_ExplosureTime->resize(CwParas.LineeditWidth, CwParas.LineeditHeight);
    ui->lineEdit_FpsSet->move(2 * _dx + CwParas.LabelWidth, 2 * _dy + CwParas.LabelHeight);
    ui->lineEdit_FpsSet->resize(CwParas.LineeditWidth, CwParas.LineeditHeight);

    ui->pushButton_ParamsSet->move(3 * _dx + CwParas.LabelWidth + CwParas.LabelWidth, 2 * _dy + CwParas.LabelHeight);
    ui->pushButton_ParamsSet->resize(CwParas.LineeditWidth, CwParas.LineeditHeight);
}

//操作框
void CameraWindow::GeoSetFrameCamOperate()
{

}

//抬头框
void CameraWindow::GeoSetFrameWinInfo()
{

}




/* **************************************相机操作框内的函数设计***********************************************/

//枚举设备按钮
void CameraWindow::on_pushButton_ListDevice_clicked()
{
    IGXFactory::GetInstance().Init();//初始化
    IGXFactory::GetInstance().UpdateDeviceList(1000, this->vectorDeviceInfo);//更新设备列表
    ui->lineEdit_NumOfCam->setText(QString::number(vectorDeviceInfo.size()));//更新现有设备数，并在窗口显示
}

//打开相机按钮
void CameraWindow::on_pushButton_OpenCam_clicked()
{
    int IndexOfCam = (ui->lineEdit_CamIndex->text()).toInt();//得到打开相机的序号
    if (vectorDeviceInfo.size() > 0 && vectorDeviceInfo.size() >= IndexOfCam && IndexOfCam >= 1) {
        GxIAPICPP::gxstring strSN = vectorDeviceInfo[IndexOfCam - 1].GetSN();
        ObjDevicePtr = IGXFactory::GetInstance().OpenDeviceBySN(strSN, GX_ACCESS_EXCLUSIVE);
        ObjStreamPtr = ObjDevicePtr->OpenStream(IndexOfCam - 1);//连接流采集通道
        ObjFeatureControlPtr = ObjDevicePtr->GetRemoteFeatureControl();//获取远端设备控制器，可用于调节相机参数
        AllocateRoomForMatCurImgPtr();//为Mat矩阵开辟空间

        //判断图像对象是否为空
        if (m_pBitmap != NULL)
        {
            delete m_pBitmap;
            m_pBitmap = NULL;
        }
        //为画图对象分配内存
        m_pBitmap = new CGXBitmap(ObjDevicePtr);//为单帧采集开辟空间
    }
}

//关闭相机按钮，使用了try来防止报错
void CameraWindow::on_pushButton_CloseCam_clicked()
{
    try {
        ObjStreamPtr->Close();
        ObjDevicePtr->Close();//关闭相机，释放资源

        if (m_pBitmap != NULL)//释放图片内存空间
        {
            delete m_pBitmap;
            m_pBitmap = NULL;
        }
    }
    catch (CGalaxyException)
    {
        QMessageBox::information(this, "提示", "关闭按钮报错，请先枚举设备，再打开相机");
    }
}
//开始采集按钮
void CameraWindow::on_pushButton_StartGrab_clicked()
{
    //进行注册回调采集函数，后期关闭相机自动释放资源
    pCaptureEventHandler = new CSampleCaptureEventHandler();
    ObjStreamPtr->RegisterCaptureCallback(pCaptureEventHandler, this);//将整个界面作为参数进行传递
    //开启流通道采集
    ObjStreamPtr->StartGrab();

    //给设备发送开采命令
    ObjFeatureControlPtr->GetCommandFeature("AcquisitionStart")->Execute();


}

//停止采集按钮,使用了try来防止报错
void CameraWindow::on_pushButton_StopGrab_clicked()
{
    try {
        ObjFeatureControlPtr->GetCommandFeature("AcquisitionStop")->Execute();
        ObjStreamPtr->StopGrab();
        //注销采集回调
        ObjStreamPtr->UnregisterCaptureCallback();

        ObjStreamPtr->Close();
        ObjDevicePtr->Close();
        IGXFactory::GetInstance().Uninit();
      

    }
    catch (CGalaxyException)
    {
        QMessageBox::information(this, "Warning", "STOP, please turn on the camera and try again");
    }
}


//单帧采集按钮
void CameraWindow::on_pushButton_GetImg_clicked()
{
    //设置采集路径
    strFilePath = ui->lineEdit_FilePath->text().toStdString();
    //设置保存文件名
    strFileName = ui->lineEdit_FileName->text().toStdString();

    m_bCheckSaveBmp = true;
    TimePushButton = clock();//开始计时
  
    



}

/* **************************************拍照参数函数设计***********************************************/
//相机参数设计
void CameraWindow::on_pushButton_ParamsSet_clicked()
{
    //设置曝光时间
    float ParmsExposureTime = (ui->lineEdit_ExplosureTime->text()).toFloat();
    ParmsExposureTime = ParmsExposureTime > 20 ? ParmsExposureTime : 20;
    ObjFeatureControlPtr->GetFloatFeature("ExposureTime")->SetValue(ParmsExposureTime);

    //设置增益
    float ParmsFps = (ui->lineEdit_FpsSet->text()).toFloat();
    ParmsFps = ParmsFps > 0.1 ? ParmsFps : 0.1;
    ObjFeatureControlPtr->GetFloatFeature("Gain")->SetValue(ParmsFps);




}




/* **************************************Opencv函数设计***********************************************/
//打开相机时调用此函数获得图片基本信息
void CameraWindow::AllocateRoomForMatCurImgPtr()
{
    ObjFeatureControlPtr = ObjDevicePtr->GetRemoteFeatureControl();
    CIntFeaturePointer ObjIntPtrWidth = ObjFeatureControlPtr->GetIntFeature("AAROIWidth");
    CIntFeaturePointer ObjIntPtrHeight = ObjFeatureControlPtr->GetIntFeature("AAROIHeight");
    ImgWidth = ObjIntPtrWidth->GetValue();//获取图片宽
    ImgHeight = ObjIntPtrHeight->GetValue();//获取图片高
    CurImgMat.create(ImgHeight, ImgWidth, CV_8UC1);//为Mat矩阵开辟空间
}

//连续采集绘图设备
void CameraWindow::ShowCurImgInLabel(QLabel* ptrLabelToShow, cv::Mat& CVMat)
{
    //获取要显示图片的label的大小
    QSize LabelSize = ptrLabelToShow->size();
    QImage QSrcImg = CV2QTFORMAT::cvMatToQImage(CVMat);//获取一个QImage
    QImage QSrcImgRatio = QSrcImg.scaled(LabelSize, Qt::IgnoreAspectRatio);//重新调整图像大小以适应窗口
    ptrLabelToShow->setPixmap(QPixmap::fromImage(QSrcImgRatio));//显示
}


