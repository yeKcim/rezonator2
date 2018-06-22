#include "MultiCausticWindow.h"

#include "../VariableDialog.h"
#include "../funcs/CausticFunction.h"
#include "../io/z_io_utils.h"
#include "../io/z_io_json.h"
#include "../widgets/Plot.h"

#include <QDebug>

MultiCausticWindow::MultiCausticWindow(Schema *schema): PlotFuncWindowStorable(new MultiCausticFunction(schema))
{
    setTitleAndIcon(function()->name(), ":/toolbar/func_multi_caustic");
}

bool MultiCausticWindow::configureInternal()
{
    auto args = function()->args();
    bool ok = VariableDialog::MultiElementRangeDlg(schema(), args, tr("Ranges"), "func_multi_caustic").run();
    if (ok)
        function()->setArgs(args);
    return ok;
}

void MultiCausticWindow::fillGraphWithFunctionResults(Z::WorkPlane plane, Graph *graph, int resultIndex)
{
    int resultIndex1 = 0;
    double offset = 0;
    for (CausticFunction* func : function()->funcs())
    {
        // Offset result for current element to place it after all previous elements
        int resultIndex2 = resultIndex1 + func->resultCount(plane);
        if (resultIndex >= resultIndex1 && resultIndex < resultIndex2)
        {
            auto result = func->result(plane, resultIndex - resultIndex1);
            graph->clearData();
            for (int i = 0; i < result.x.size(); i++)
                graph->addData(result.x.at(i) + offset, result.y.at(i));
            return;
        }
        resultIndex1 = resultIndex2;
        offset += func->argumentUnit()->fromSi(func->arg()->range.stop.toSi());
    }
}

void MultiCausticWindow::afterUpdate()
{
    updateElementBoundMarkers();
}

void MultiCausticWindow::updateElementBoundMarkers()
{
    double offset = 0;
    QList<QCPItemStraightLine*> markers;
    auto funcs = function()->funcs();
    for (int i = 0; i < funcs.size()-1; i++)
    {
        offset += funcs.at(i)->argumentUnit()->fromSi(funcs.at(i)->arg()->range.stop.toSi());
        QCPItemStraightLine* marker;
        if (!_elemBoundMarkers.isEmpty())
        {
            marker = _elemBoundMarkers.first();
            _elemBoundMarkers.removeFirst();
        }
        else marker = makeElemBoundMarker();
        marker->point1->setCoords(offset, 0);
        marker->point2->setCoords(offset, 1);
        markers.append(marker);
    }
    for (auto oldMarker : _elemBoundMarkers)
        plot()->removeItem(oldMarker);
    _elemBoundMarkers = markers;
}

QCPItemStraightLine* MultiCausticWindow::makeElemBoundMarker() const
{
    QCPItemStraightLine *line = new QCPItemStraightLine(plot());
    line->setPen(QPen(Qt::magenta, 1, Qt::DashLine)); // TODO make configurable
    line->setSelectable(false);
    plot()->addItem(line);
    return line;
}
