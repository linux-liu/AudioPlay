## 本播放器使用ffmpeg+opensl es+soundtouch来实现音频播放器的开发，实现的功能包括基本的播放、暂停、seek、声道切换、变速、变调、静音、计算声音分贝值、音频数据本身音量的调节等功能。
### 下图为实现的功能图
![image](https://github.com/linux-liu/AudioPlay/blob/master/122.jpg)

###使用方法
主要使用LXPlayer这个类,如下
```java
 lxPlayer = LXPlayer.getInstance();
 //准备数据
  lxPlayer.prepare("http://od.qingting.fm/m4a/5cfbe4377cb8915f99370adb_12803144_24.m4a");
  //准备开始回调，这里实现了很多回调，所有的回调都是在主线程中
  lxPlayer.setOnPrepareListener(new OnPrepareListener() {
            @Override
            public void prepare() {
               //开始播放
                lxPlayer.start();

            }
        });

//其他方法和回调
 lxPlayer.pause(); //暂停
 lxPlayer.play();//继续播放
  lxPlayer.seek(currentSec);//定位到某一位置，以秒为单位
 
 lxPlayer.release();//释放播放资源
 lxPlayer.setPitch(1.5); //变调
 lxPlayer.setTemPo(1.5); //变速
 lxPlayer.setMute(true);//是否静音
 lxPlayer.setChannelSolo(0)；设置声道 0左声道 1右声道 2立体声
 lxPlayer.getDuration()//获取时长
 lxPlayer.nextOrPre(String url);//上一首或者下一首
 
 //回调方法包括播放完成回调，音频的播放分贝回调，出错回调，加载回调，播放状态回调，播放进度回调。
  lxPlayer.setOnCompleteListener(new OnCompleteListener() {
            @Override
            public void onComplete() {
                Log.e("ffmpeg", "完成");
            }
        });
        
  lxPlayer.setOnDBListener(new OnDBListener() {
            @Override
            public void currentDB(int db) {
               Log.e("ffmpeg", "当前的db==>" + db);
            }
        });       
        
  lxPlayer.setOnErrorListener(new OnErrorListener() {
            @Override
            public void error(int code, String msg) {
                Log.e("ffmepg", "error code==>" + code + " msg==>" + msg + "thread=>" + Thread.currentThread().getName());
            }
        });
        
          lxPlayer.setOnLoadListener(new OnLoadListener() {
            @Override
            public void onLoad(boolean isLoad) {
               //isLoad为true代表正在加载
                Log.e("ffmpeg","isLoad==>"+isLoad);
            }
        });
        
         lxPlayer.setOnPlayStatusListener(new OnPlayStatusListener() {

            @Override
            public void playStatus(final int status) {
                if (status == 1) {
                    tv.setText("播放状态");
                } else if (status == 2) {
                    tv.setText("暂停状态");
                } else if (status == 3) {
                    tv.setText("停止状态");
                } else if (status == 0) {
                    tv.setText("初始状态");
                }


            }
        });
        
         lxPlayer.setOnProgressListener(new OnProgressListener() {
            @Override
            public void progress(int duration, int current) {

                if (!isTouch) {
                    final int min = duration / 60;
                    final int sec = duration % 60;
                    final int cumin = current / 60;
                    final int cusec = current % 60;
                    textView.setText(cumin + ":" + cusec + "/" + min + ":" + sec);
                    if (duration != 0) {
                        seekBar.setProgress(current * 100 / duration);
                    } else {
                        seekBar.setProgress(0);
                    }


                }


            }
        });
        
            

        
```
更详细的用法可以查看demo中的使用。这里也开放了源代码，有任何问题可以提issue。
