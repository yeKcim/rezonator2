#include "FormulaEditor.h"
#include "ParamEditor.h"
#include "helpers/OriLayouts.h"
#include "helpers/OriWidgets.h"

#include <cassert>

#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QTextEdit>
#include <QTimer>
#include <QPushButton>

FormulaEditor::FormulaEditor(Z::Parameter *param, Z::Formulas *formulas, QWidget *parent) : QWidget(parent)
{
    _param = param;
    _tmpParam = new Z::Parameter(param->dim(), param->alias());
    _tmpParam->setValue(param->value());

    _formulas = formulas;

    _formula = _formulas->get(_param);
    _hasFormula = _formula;

    auto menu = new QMenu(this);
    _actnAddFormula = menu->addAction(tr("Add formula"), this, &FormulaEditor::addFormula);
    _actnRemoveFormula = menu->addAction(tr("Remove formula"), this, &FormulaEditor::removeFormula);
    _actnRemoveFormula->setVisible(false);

    _paramEditor = new ParamEditor(_tmpParam);

    auto optionsButton = new QPushButton;
    optionsButton->setFlat(true);
    optionsButton->setIcon(QIcon(":/toolbar/options"));
    optionsButton->setFixedWidth(24);
    connect(optionsButton, &QPushButton::clicked, [menu, optionsButton](){
        // button->setMenu() crashes the app on MacOS when button is clicked, so show manually
        menu->popup(optionsButton->mapToGlobal(optionsButton->rect().bottomLeft()));
    });

    Ori::Layouts::LayoutV({
        Ori::Layouts::LayoutH({
            _paramEditor,
            optionsButton
        })
        .setMargin(0)
        .setSpacing(0)
    })
    .setMargin(0)
    .useFor(this);

    toggleFormulaView();

    if (_hasFormula)
        _codeEditor->setFocus();

    // NOTE: _tmpParam must be parented after _paramEditor to be deleted in last turn.
    // While it's ok on GCC to delete _tmpParam manually in ~FormulaEditor(), but on LLVM (MacOS)
    // this destructor seems to be called BEFORE parent's one, and _tmpParam is freed too early.
    new OwnedPayload<Z::Parameter>(_tmpParam, this);
}

void FormulaEditor::addFormula()
{
    _hasFormula = true;
    toggleFormulaView();
    _codeEditor->setFocus();
}

void FormulaEditor::removeFormula()
{
    _hasFormula = false;
    toggleFormulaView();
    _paramEditor->setFocus();
}

void FormulaEditor::createCodeEditor()
{
    enum { ROW_VALUE, ROW_CODE, ROW_STATUS };

    _tmpFormula = new Z::Formula(_tmpParam);
    new OwnedPayload<Z::Formula>(_tmpFormula, this);
    if (_formula)
        _tmpFormula->setCode(_formula->code());

    assert(_codeEditor == nullptr);
    _codeEditor = new QTextEdit;
    _codeEditor->setAcceptRichText(false);
    _codeEditor->setPlainText(_tmpFormula->code());
    Ori::Gui::setFontMonospace(_codeEditor);
    qobject_cast<QVBoxLayout*>(layout())->insertWidget(ROW_CODE, _codeEditor);
    connect(_codeEditor, &QTextEdit::textChanged, this, &FormulaEditor::formulaCodeChanged);

    assert(_formulaStatus == nullptr);
    _formulaStatus = new QLabel;
    _formulaStatus->setWordWrap(true);
    qobject_cast<QVBoxLayout*>(layout())->insertWidget(ROW_STATUS, _formulaStatus);

    assert(_recalcTimer == nullptr);
    _recalcTimer = new QTimer(this);
    _recalcTimer->setInterval(250);
    _recalcTimer->setSingleShot(true);
    connect(_recalcTimer, &QTimer::timeout, this, &FormulaEditor::calculateFormula);
}

void FormulaEditor::toggleFormulaView()
{
    if (_hasFormula && !_tmpFormula)
        createCodeEditor();

    if (_codeEditor)
        _codeEditor->setVisible(_hasFormula);
    if (_formulaStatus)
        _formulaStatus->setVisible(_hasFormula);

    _actnAddFormula->setVisible(!_hasFormula);
    _actnRemoveFormula->setVisible(_hasFormula);
    qobject_cast<QLineEdit*>(_paramEditor->valueEditor())->setReadOnly(_hasFormula);

    if (_hasFormula)
    {
        _tmpFormula->calculate();
        showFormulaStatus();
    }
}

void FormulaEditor::apply()
{
    if (_hasFormula)
    {
        if (!_formula)
        {
            _formula = new Z::Formula(_param);
            _formulas->put(_formula);
        }
        _formula->setCode(_tmpFormula->code());
    }
    else
    {
        if (_formula)
        {
            _formulas->free(_param);
            _formula = nullptr;
        }
        _paramEditor->apply();
    }
    _param->setValue(_tmpParam->value());
}

void FormulaEditor::calculateFormula()
{
    _tmpFormula->setCode(_codeEditor->toPlainText());
    _tmpFormula->calculate();
    showFormulaStatus();
}

void FormulaEditor::formulaCodeChanged()
{
    _recalcTimer->start();
}

void FormulaEditor::showFormulaStatus()
{
    if (_tmpFormula->ok())
    {
        _formulaStatus->setText("OK");
        _formulaStatus->setStyleSheet("QLabel{background:LightGreen;padding:3px}");
    }
    else
    {
        _formulaStatus->setText(_tmpFormula->status());
        _formulaStatus->setStyleSheet("QLabel{background:LightCoral;padding:3px}");
    }
}
