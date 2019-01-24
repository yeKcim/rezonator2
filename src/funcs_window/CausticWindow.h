#ifndef CAUSTIC_WINDOW_H
#define CAUSTIC_WINDOW_H

#include "PlotFuncWindowStorable.h"
#include "../RezonatorDialog.h"
#include "../funcs/CausticFunction.h"

class ElemSelectorWidget;
namespace VariableRangeEditor {
    class PointsRangeEd;
}

class CausticWindow : public PlotFuncWindowStorable
{
    Q_OBJECT

public:
    explicit CausticWindow(Schema*);

    CausticFunction* function() const { return dynamic_cast<CausticFunction*>(_function); }

protected:
    // Implementation of PlotFuncWindow
    QWidget* makeOptionsPanel() override;
    bool configureInternal() override;

    // Implementation of PlotFuncWindowStorable
    QString readFunction(const QJsonObject& root) override;
    QString writeFunction(QJsonObject& root) override;
};


/**
    The function arguments dialog that can choose one of range elements
    and set the number of points for plotting inside of a selected element.
*/
class CausticParamsDlg : public RezonatorDialog
{
    Q_OBJECT

public:
    explicit CausticParamsDlg(Schema*, Z::Variable*);

protected slots:
    void collect();

private slots:
    void guessRange();

private:
    Z::Variable* _var;
    ElemSelectorWidget* _elemSelector;
    VariableRangeEditor::PointsRangeEd* _rangeEditor;
    QString _recentKey = "func_caustic";

    void populate();
};

#endif // CAUSTIC_WINDOW_H
