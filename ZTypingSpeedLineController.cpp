//===================================================
#include "ZTypingSpeedLineController.h"
//===================================================
ZTypingSpeedLineController::ZTypingSpeedLineController(ZTask &task, QObject *parent)
    : ZAbstractLineController{task, parent}
{

}
//===================================================
QString ZTypingSpeedLineController::zp_nextLine(bool *completed)
{

    return "typing Speed";
}
//===================================================
void ZTypingSpeedLineController::zp_reset()
{

}
//===================================================

