#include "StabilityMapWindow.h"

#include "../CustomPrefs.h"
#include "../io/z_io_utils.h"
#include "../io/z_io_json.h"
#include "../VariableDialog.h"
#include "../widgets/Plot.h"
#include "helpers/OriLayouts.h"

#include <QAction>

class StabilityMapOptionsPanel : public FuncOptionsPanel
{
public:
    StabilityMapOptionsPanel(StabilityMapWindow* window) : FuncOptionsPanel(window), _window(window)
    {
        // TODO: check if these strings are translated
        Ori::Layouts::LayoutV({
            makeSectionHeader(tr("Stability parameter")),
            makeModeButton(":/toolbar/formula_stab_normal", tr("Normal"), int(Z::Enums::StabilityCalcMode::Normal)),
            makeModeButton(":/toolbar/formula_stab_squared", tr("Squared"), int(Z::Enums::StabilityCalcMode::Squared)),
            Ori::Layouts::Stretch()
        }).setSpacing(0).setMargin(0).useFor(this);

        for (auto button : _modeButtons)
            button->setIconSize(QSize(96, 48));

        showCurrentMode();
    }

    int currentFunctionMode() const override
    {
        return static_cast<int>(_window->function()->stabilityCalcMode());
    }

    void functionModeChanged(int mode) override
    {
        auto stabCalcMode = static_cast<Z::Enums::StabilityCalcMode>(mode);
        CustomPrefs::setRecentStr(QStringLiteral("func_stab_map_mode"), Z::Enums::toStr(stabCalcMode));
        _window->function()->setStabilityCalcMode(stabCalcMode);
    }

private:
    StabilityMapWindow* _window;
};

//------------------------------------------------------------------------------
//                            StabilityMapWindow
//------------------------------------------------------------------------------

StabilityMapWindow::StabilityMapWindow(Schema *schema) :
    PlotFuncWindowStorable(new StabilityMapFunction(schema))
{
    setTitleAndIcon(function()->name(), ":/toolbar/func_stab_map");
    createControl();
}

void StabilityMapWindow::createControl()
{
    actnStabilityAutolimits = new QAction(tr("Y-axis -> Stability Range", "Plot action"), this);
    actnStabilityAutolimits->setIcon(QIcon(":/toolbar/limits_stab"));
    connect(actnStabilityAutolimits, SIGNAL(triggered()), this, SLOT(autolimitsStability()));

    menuLimits->addSeparator();
    menuLimits->addAction(actnStabilityAutolimits);

    toolbar()->addSeparator();
    toolbar()->addAction(actnStabilityAutolimits);
}

bool StabilityMapWindow::configureInternal()
{
    return VariableDialog::ElementDlg(schema(), function()->arg(), tr("Variable"), "func_stab_map").run();
}

void StabilityMapWindow::autolimitsStability()
{
    switch (function()->stabilityCalcMode())
    {
    case Z::Enums::StabilityCalcMode::Normal:
        _plot->setLimitsY(-1.05, 1.05);
        break;

    case Z::Enums::StabilityCalcMode::Squared:
        _plot->setLimitsY(-0.05, 1.05);
        break;
    }
}

QWidget* StabilityMapWindow::makeOptionsPanel()
{
    return new StabilityMapOptionsPanel(this);
}

QString StabilityMapWindow::readFunction(const QJsonObject& root)
{
    function()->setStabilityCalcMode(Z::IO::Utils::enumFromStr(
        root["stab_calc_mode"].toString(), Z::Enums::StabilityCalcMode::Normal));
    auto res = Z::IO::Json::readVariable(root["arg"].toObject(), function()->arg(), schema());
    if (!res.isEmpty())
        return res;
    return QString();
}

QString StabilityMapWindow::writeFunction(QJsonObject& root)
{
    root["stab_calc_mode"] = Z::IO::Utils::enumToStr(function()->stabilityCalcMode());
    root["arg"] = Z::IO::Json::writeVariable(function()->arg(), schema());
    return QString();
}
