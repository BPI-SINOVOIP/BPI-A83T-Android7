#include "hwc.h"
#include <cutils/list.h>

void createList(struct listnode *list) {
    list_init(list);
}

void showLayer(struct Layer *lay) {
    int count = 0;
    char buf[400] = {0};

    if (NULL == lay) {
        ALOGE("layer is NULL!\n");
        return;
    }
    count += sprintf(buf + count, " layer[%p] ", lay);
    count += sprintf(buf + count, " handle[%1p] ", lay->buffer);
    count += sprintf(buf + count, " acquirefence[%d] ", lay->acquireFence);
    count += sprintf(buf + count, " releaseFence[%d] ", lay->releaseFence);
    count += sprintf(buf + count, " composition type: [%d] ", lay->compositionType);
    count += sprintf(buf + count, " mode[%3d] ", lay->blendMode);
    count += sprintf(buf + count, " dspace[%d] ", lay->dataspace);
    count += sprintf(buf + count, " transform[%d] ", lay->transform);
    count += sprintf(buf + count, " frame[%4d,%4d,%4d,%4d] ", lay->frame.left,
            lay->frame.top, lay->frame.right, lay->frame.bottom);
    count += sprintf(buf + count, " planeAlpha[%4f] ", lay->planeAlpha);
    count += sprintf(buf + count, " crop[%4f, %4f, %4f, %4f] ", lay->crop.left,
            lay->crop.top, lay->crop.right, lay->crop.bottom);
    count += sprintf(buf + count, " tr[%1d] ", lay->transform);
    //count += sprintf(buf + count, " vi[%1d] ", lay->visibleRegion);
    count += sprintf(buf + count, " z[%1d] ", lay->zorder);
    count += sprintf(buf + count, " changed[%1d] \n", lay->typeChange);
    ALOGV("%s", buf);
}

void printList(struct listnode *list){
    struct listnode *node;

    list_for_each(node, list) {
        struct Layer *lay = node_to_item(node, struct Layer, node);
        showLayer(lay);
    }
}

void clearList(struct listnode *list) {
    struct listnode *node;

    while (!list_empty(list)) {
        node = list_head(list);
        list_remove(node);
        free(node_to_item(node, struct Layer, node));
    }
}

void clearListExcepFbTarget(struct listnode *list) {
    struct listnode *node;
    struct Layer *lay;

    int size = sizeList(list);
    while (!list_empty(list) && size--) {
        node = list_head(list);
        lay = node_to_item(node, struct Layer, node);
        if (lay->compositionType == HWC2_COMPOSITION_CLIENT_TARGET) {
            continue;
        }
        list_remove(node);
        free(node_to_item(node, struct Layer, node));
    }
}

int sizeList(struct listnode *list) {
    int ret = 0;
    struct listnode *node;
    list_for_each(node, list) {
        ret++;
    }
    return ret;
}

bool isEmptyList(struct listnode *list) {
    return list_empty(list);
}

void insertLayerByZorder(Layer *element, struct listnode *list) {
    struct listnode *node;
    struct Layer *lay;

    if (list_empty(list)) {
        list_add_tail(list, &element->node);
        return;
    }

    list_remove(&element->node);
    list_for_each(node, list) {
        lay = node_to_item(node, struct Layer, node);
        if (lay->zorder > element->zorder){
            break;
        }else if (lay->zorder == element->zorder){
            ALOGV("warning: there is same zoder in layer list.\n");
        }
    }
    list_add_tail(&(lay->node), &(element->node));
}

bool removeLayer(Layer *element) {
    if(element->node.next != &(element->node) && element->node.prev != &(element->node)){
        list_remove(&element->node);
        free(node_to_item(&element->node, struct Layer, node));
        return true;
    }else{
        free(node_to_item(&element->node, struct Layer, node));
        ALOGD("Warning: the layer %p is not in the list", element);
        return false;
    }
}
