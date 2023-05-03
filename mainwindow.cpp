#pragma execution_character_set("utf-8")

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "qfiledialog.h"
#include "qmessagebox.h"

#include <QDebug>

//#include "libflv/mpeg4-avc.h"
//#include "libflv/flv-muxer.h"
//#include "libflv/flv-writer.h"
#include "3rdparty/inlcude/libflv/flv-muxer.h"
#include "3rdparty/inlcude/libflv/flv-writer.h"
#include "3rdparty/inlcude/libflv/mpeg4-avc.h"

struct h264_raw_t
{
    flv_muxer_t* flv;
    int audioType;
    void* rtp_video_encoder, * rtp_audio_encoder;
    struct ps_muxer_t* psenc;
    int aid, vid;
    uint32_t pts, dts;
    const uint8_t* ptr;
    const uint8_t* aptr;
    int vcl;
};

enum {
    NONEVIDEO = 0,
    H264,
    H265
} VideoType;

enum {
    NONEAUDIO = 0,
    G711A,
    G711U,
    MP3,
    AAC
} AudioType;

enum {
    FLV = 0,
    MP4
} OutFileType;

static int on_flv_packet(void* flv, int type, const void* data, size_t bytes, uint32_t timestamp)
{
    return flv_writer_input(flv, type, data, bytes, timestamp);
}

static void flv_h264_handler(void* param, const uint8_t* nalu, size_t bytes)
{
    struct h264_raw_t* ctx = (struct h264_raw_t*)param;
    assert(ctx->ptr < nalu);

    const uint8_t* ptr = nalu - 3;
    uint8_t nalutype = nalu[0] & 0x1f;
    if (ctx->vcl > 0 && h264_is_new_access_unit((const uint8_t*)nalu, bytes)) {
        flv_muxer_avc(ctx->flv, ctx->ptr, ptr - ctx->ptr, ctx->pts, ctx->dts);
        if (ctx->audioType == G711U) {
            flv_muxer_g711u(ctx->flv, ctx->aptr, 320, ctx->pts, ctx->dts);
        } else if (ctx->audioType == G711A) {
            flv_muxer_g711a(ctx->flv, ctx->aptr, 320, ctx->pts, ctx->dts);
        } else {
            // 没有音频
        }
        ctx->pts += 40;
        ctx->dts += 40;

        ctx->ptr = ptr;
        ctx->aptr += 320;
        ctx->vcl = 0;
    }

    if (1 <= nalutype && nalutype <= 5) {
        ++ctx->vcl;
    }
}

void MainWindow::avc2flv_test()
{
    char outputFile[10] = "out.flv";

    struct h264_raw_t ctx;
    memset(&ctx, 0x00, sizeof(ctx));

    void* f = flv_writer_create(outputFile);
    ctx.flv = flv_muxer_create(on_flv_packet, f);

    QFile videoFile(m_videoFile);
    videoFile.open(QIODevice::ReadOnly);
    QByteArray videoArr = videoFile.readAll();

    QFile audioFile(m_audioFile);
    audioFile.open(QIODevice::ReadOnly);
    QByteArray audioArr = audioFile.readAll();
    qDebug() << "audio type:" << m_audio_type;
    ctx.audioType = m_audio_type;

    ctx.ptr = (uint8_t*)videoArr.data();
    ctx.aptr = (uint8_t*)audioArr.data();
    if (m_video_type == H264) {
        mpeg4_h264_annexb_nalu(videoArr.data(), videoArr.length(), flv_h264_handler, &ctx);
    } else if (m_video_type == H265) {

    } else {
        qDebug() << "don't support the video type";
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    groupButton1.addButton(ui->btnH264Type, 0);
    groupButton1.addButton(ui->btnH265Type, 1);

    groupButton2.addButton(ui->btnG711AType, 0);
    groupButton2.addButton(ui->btnG711UType, 1);

    groupOutButton.addButton(ui->btnFlvType, 0);
    groupOutButton.addButton(ui->btnMp4Type, 1);

    connect(ui->btnH264Type, SIGNAL(clicked(bool)), this, SLOT(slots_video_check()));
    connect(ui->btnH265Type, SIGNAL(clicked(bool)), this, SLOT(slots_video_check()));

    connect(ui->btnG711AType, SIGNAL(clicked(bool)), this, SLOT(slots_audio_check()));
    connect(ui->btnG711UType, SIGNAL(clicked(bool)), this, SLOT(slots_audio_check()));

    connect(ui->btnFlvType, SIGNAL(clicked(bool)), this, SLOT(slots_outFile_check()));
    connect(ui->btnMp4Type, SIGNAL(clicked(bool)), this, SLOT(slots_outFile_check()));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_btnOpenVideoFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "选择文件", "", "视频(*.h264 *.h265)");
    if (!fileName.isEmpty()) {
        ui->txtVideoFile->setText(fileName);
        m_videoFile = fileName;
    }
}

void MainWindow::on_btnOpenAudioFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "选择文件", "", "视频(*.pcma *.pcmu)");
    if (!fileName.isEmpty()) {
        ui->txtAudioFile->setText(fileName);
        m_audioFile = fileName;
    }
}

void MainWindow::on_btnOutMediaFile_clicked()
{
    if (m_videoFile.length() == 0 || m_audioFile.length() == 0) {
        QMessageBox errBox(QMessageBox::Question,"提示","请选择要合成的文件",QMessageBox::Close);
        errBox.exec();
        return;
    }

    if (m_outFile_type == FLV) {
        avc2flv_test();
    } else if (m_outFile_type == MP4) {

    } else {

    }

    QMessageBox MyBox(QMessageBox::Question,"提示","合成成功",QMessageBox::Close);
    MyBox.exec();
}

void MainWindow::on_btnClear_clicked()
{
    ui->txtVideoFile->clear();
    ui->txtAudioFile->clear();
}

void MainWindow::slots_video_check()
{
    switch(groupButton1.checkedId()) {
    case 0:
        m_video_type = H264;
        break;
    case 1:
        m_video_type = H265;
        break;

    default:
        break;
    }
    qDebug() << "check video type is " << m_video_type;
}

void MainWindow::slots_audio_check()
{
    switch(groupButton2.checkedId()) {
    case 0:
        m_audio_type = G711A;
        break;
    case 1:
        m_audio_type = G711U;
        break;

    default:
        break;
    }

    qDebug() << "check audio type is " << m_audio_type;
}

void MainWindow::slots_outFile_check()
{
    switch(groupOutButton.checkedId()) {
    case 0:
        m_outFile_type = FLV;
        break;
    case 1:
        m_outFile_type = MP4;
        break;

    default:
        break;
    }

    qDebug() << "check out type is " << m_outFile_type;
}
