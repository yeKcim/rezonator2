#ifndef ELEMENT_FORMULA_H
#define ELEMENT_FORMULA_H

#include "Element.h"

#include <QApplication>

DECLARE_ELEMENT(ElemFormula, Element)
    ElemFormula();
    TYPE_NAME(qApp->translate("Elements", "Formula element"))
    DEFAULT_LABEL("C")
    CALC_MATRIX
    bool hasMatricesTS() const { return _hasMatricesTS; }
    void setHasMatricesTS(bool on) { _hasMatricesTS = on; }
    QString formula() const { return _formula; }
    QString error() const { return _error; }
    void setFormula(const QString& formula) { _formula = formula; }
    void addParam(Z::Parameter* param, int index = -1);
    void removeParam(Z::Parameter* param);
    void moveParamUp(Z::Parameter* param);
    void moveParamDown(Z::Parameter* param);
    void assign(const ElemFormula* other);
private:
    bool _hasMatricesTS = true;
    QString _formula;
    QString _error;
DECLARE_ELEMENT_END

#endif // ELEMENT_FORMULA_H
