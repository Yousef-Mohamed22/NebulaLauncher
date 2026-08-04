import QtQuick

Image {

    sourceSize.width: width * Screen.devicePixelRatio
    sourceSize.height: height * Screen.devicePixelRatio
    smooth: true
    mipmap: true
    asynchronous: true

}
