#include "MultiCausticWindow.h"

#include "../io/z_io_utils.h"
#include "../io/z_io_json.h"
#include "../VariableDialog.h"

MultiCausticWindow::MultiCausticWindow(Schema *schema): PlotFuncWindowStorable(new MultiCausticFunction(schema))
{
    setTitleAndIcon(function()->name(), ":/toolbar/func_multi_caustic");
}

bool MultiCausticWindow::configureInternal()
{
    return VariableDialog::MultiElementRangeDlg(schema()/*, function()->arg()*/, tr("Ranges"), "func_multi_caustic").run();
}
