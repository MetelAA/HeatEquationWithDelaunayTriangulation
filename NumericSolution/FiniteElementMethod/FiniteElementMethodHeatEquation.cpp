

#include "FiniteElementMethodHeatEquation.h"

FiniteElementMethodHeatEquation::FiniteElementMethodHeatEquation(const std::vector<Point_2> &points,
                                                                 const std::vector<DTO::TriangleFace> &trFaces,
                                                                 const std::vector<BoundaryNode> &boundaryNodes,
                                                                 double initTemperature, double dt,
                                                                 double thermalConductivityCoefficient) {
    //в этот конструктор попадают уже только внутренние грани, т.е. отсеяные от того что лишним образом натреагулировали
    //boundaryNodes - все граничные ноды, нужно где-то до этого напроецировать на них их температуры, не охота это здесь разводить учитывая что у нас есть список этих вершин на других, более ранних шагах
    this->dt = dt;
    this->thermalConductivityCoefficient = thermalConductivityCoefficient;


    A = std::vector<std::vector<double>>(points.size(), std::vector<double>(points.size(), 0));
    std::vector<std::vector<double>> K(points.size(), std::vector<double>(points.size(), 0));
    M = std::vector<std::vector<double>>(points.size(), std::vector<double>(points.size(), 0));
    //булем считать локальные матрицы для каждого элемента
    for(DTO::TriangleFace face : trFaces){
        std::vector<std::vector<double>> local_K;
        std::vector<std::vector<double>> local_M;
        {
            Point_2 i = points[face.v1_index];
            Point_2 j = points[face.v2_index];
            Point_2 k = points[face.v3_index];

            double s = i.getX() * (j.getY() - k.getY()) + j.getX() * (k.getY() - i.getY()) + k.getX() * (i.getY() - j.getY()); //площадь * 2

            double b_i, c_i; //vertex i, a_* - нет тк они откисают на этапе применения градиента
            b_i = j.getY() - k.getY();
            c_i = k.getX() - j.getX();

            double b_j, c_j;
            b_j = k.getY() - i.getY();
            c_j = i.getX() - k.getX();

            double b_k, c_k;
            b_k = i.getY() - j.getY();
            c_k = j.getX() - i.getX();


            {
                double multiplier = 1.0 / (2*s);
                local_K = {
                        {multiplier * (std::pow(b_i, 2) + std::pow(c_i, 2)), multiplier * (b_i*b_j + c_i*c_j), multiplier * (b_i*b_k + c_i*c_k) },
                        {multiplier * (b_j*b_i + c_j*c_i), multiplier * (std::pow(b_j, 2) + std::pow(c_j, 2)), multiplier * (b_j*b_k + c_j*c_k) },
                        {multiplier * (b_k*b_i + c_k*c_i), multiplier * (b_k*b_j + c_k*c_j), multiplier * (std::pow(b_k, 2) + std::pow(c_k, 2)) }
                };
            }


            {
                double m = s / 2.0 / 12.0; //multiplier
                local_M = {
                        {m * 2, m, m},
                        {m, m * 2, m},
                        {m, m,  m * 2}
                };
            }
        }

        //теперь складываем в глобальные матрицы
        
        std::vector<size_t> localIndexToGlobalIndex = {face.v1_index, face.v2_index, face.v3_index};

        for (int i = 0; i < local_K.size(); ++i) {
            for (int j = 0; j < local_K[0].size(); ++j) {
                K[localIndexToGlobalIndex[i]][localIndexToGlobalIndex[j]] += local_K[i][j];
                M[localIndexToGlobalIndex[i]][localIndexToGlobalIndex[j]] += local_M[i][j];
            }
        }
    }

    for (int i = 0; i < M.size(); ++i) {
        for (int j = 0; j < M[0].size(); ++j) {
            A[i][j] = M[i][j] + dt * thermalConductivityCoefficient * K[i][j];
        }
    }

    std::vector<std::vector<double>> A_mod = A;

    //введём условия Дирихле, т.е. обнулим строки/столбцы для граничных нод
    for(BoundaryNode boundary : boundaryNodes){
        for (int i = 0; i < A_mod.size(); ++i) {
            bool isDiagonalElement = i == boundary.node_index;
            A_mod[i][boundary.node_index] = isDiagonalElement;
            A_mod[boundary.node_index][i] = isDiagonalElement;
        }
    }
    
    //заполним матрицы L, и L_t

    L = std::vector<std::vector<double>>(points.size(), std::vector<double>(points.size(), 0));
    {
        for (int j = 0; j < A_mod.size(); ++j) {
            double squareSum = 0;
            for (int k = 0; k < j; ++k) {
                squareSum += L[j][k] * L[j][k];
            }
            L[j][j] = std::sqrt(A_mod[j][j] - squareSum);

            for (int i = j + 1; i < A_mod.size(); ++i) {
                double multSum = 0;
                for (int k = 0; k < j; ++k) {
                    multSum += L[i][k] * L[j][k];
                }
                L[i][j] = (A_mod[i][j] - multSum) / L[j][j];
            }
        }
    }
    
    //закинем начальные условия в класс нод
    std::vector<double> nodeTemperatures(points.size());
    for (int i = 0; i < nodeTemperatures.size(); ++i) {
        nodeTemperatures[i] = initTemperature;
    }

    std::vector<bool> isBoundary(points.size(), false);
    for(BoundaryNode boundary : boundaryNodes){
        nodeTemperatures[boundary.node_index] = boundary.temp;
        isBoundary[boundary.node_index] = true;
    }

    nodes = Nodes(points, nodeTemperatures, isBoundary);
    curTime = 0;
}

void FiniteElementMethodHeatEquation::step() {
    std::vector<double> b(this->nodes.getNodesSize()); //вектор правой стороны СЛАУ
    for (int i = 0; i < this->nodes.getNodesSize(); ++i) {
        //умножаем матрицу M (n*n) на вектора температур (n*1)
        double right_element = 0;
        for (int j = 0; j < this->nodes.getNodesSize(); ++j) {
            right_element += M[i][j] * nodes.getNode(j).temp;
        }
        b[i] = right_element;
    }

    //учтём граничные условия (корректируем правую часть)
    for (size_t boundaryNodeIndex : this->nodes.getBoundaryNodesIndexes()) {
        double Tb = this->nodes.getNode(boundaryNodeIndex).temp;
        for (int i = 0; i < this->nodes.getNodesSize(); ++i) {
            b[i] -= A[i][boundaryNodeIndex] * Tb;
        }
    }
    for (size_t boundaryNodeIndex : this->nodes.getBoundaryNodesIndexes()) {
        b[boundaryNodeIndex] = this->nodes.getNode(boundaryNodeIndex).temp;
    }

    //переходим к первому этапу разложения Холецкого Ly=b, ищем вектор y-ков, пользуемся что L - нижнетреугольная
    std::vector<double> y(this->nodes.getNodesSize());

    for (int i = 0; i < this->nodes.getNodesSize(); ++i) {
        double otherSum = 0;
        for (int j = 0; j < i ; ++j) {
            otherSum += L[i][j] * y[j];
        }
        y[i] = (b[i] - otherSum) / L[i][i];
    }

    //вторая часть Холецкого L_t*x=y. Помним чо L_t - верхнетреугольная, работаем с L как с транспонированной
    std::vector<double> x(this->nodes.getNodesSize());

    for (int i = this->nodes.getNodesSize() - 1; i >= 0; --i) {
        double otherSum = 0;
        for (int j = i + 1; j < this->nodes.getNodesSize(); ++j) {
            otherSum += L[j][i] * x[j];
        }
        x[i] = (y[i] - otherSum) / L[i][i];
    }

    nodes.changeTempInNodes(x);
    curTime += dt;
}

double FiniteElementMethodHeatEquation::getCurrentTime() const {
    return curTime;
}

const FiniteElementMethodHeatEquation::Nodes & FiniteElementMethodHeatEquation::getNodesDataAccessInterface() const {
    return this->nodes;
}