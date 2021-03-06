// @flow
import * as React from 'react';
import ReactDOM from 'react-dom';
import ContextMenu from './ContextMenu';

type Props = {|
  element: React$Element<any>,
  buildMenuTemplate: () => Array<any>,
|};

type State = {||};

/**
 * Wrap an element and display a menu when `onClick` prop is called on the element.
 */
export default class ElementWithMenu extends React.Component<Props, State> {
  _contextMenu: ?ContextMenu;
  _wrappedElement: ?any;

  open = (event: any) => {
    const { _contextMenu } = this;
    if (!_contextMenu) return;

    const node = ReactDOM.findDOMNode(this._wrappedElement);
    if (node instanceof HTMLElement) {
      const dimensions = node.getBoundingClientRect();

      _contextMenu.open(
        Math.round(dimensions.left),
        Math.round(dimensions.top + dimensions.height)
      );
    }
  };

  render() {
    const { element, buildMenuTemplate } = this.props;
    return (
      <React.Fragment>
        {React.cloneElement(element, {
          onClick: this.open,
          ref: wrappedElement => (this._wrappedElement = wrappedElement),
        })}
        <ContextMenu
          ref={contextMenu => (this._contextMenu = contextMenu)}
          buildMenuTemplate={buildMenuTemplate}
        />
      </React.Fragment>
    );
  }
}
