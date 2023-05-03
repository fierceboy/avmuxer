#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    void avc2flv_test();

private slots:
    void on_btnOpenVideoFile_clicked();

    void on_btnOpenAudioFile_clicked();

    void on_btnOutMediaFile_clicked();

    void on_btnClear_clicked();

public slots:
    void slots_video_check();
    void slots_audio_check();
    void slots_outFile_check();

private:
    Ui::MainWindow *ui;

    QString m_videoFile;
    QString m_audioFile;
    QString m_outMediaFile;

    int m_audio_type;
    int m_video_type;
    int m_outFile_type;

    QButtonGroup groupButton1;
    QButtonGroup groupButton2;
    QButtonGroup groupOutButton;
};
#endif // MAINWINDOW_H
